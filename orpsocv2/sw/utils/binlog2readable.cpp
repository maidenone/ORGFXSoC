// ----------------------------------------------------------------------------

// Interprets ORPSoC's Cycle Accurate model binary format log files

// Contributor Julius Baxter <jb@orsoc.se>

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ----------------------------------------------------------------------------

// $Id$


#include <string.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <stdint.h> 
#include <fstream>
#include <sys/types.h>

using namespace std;


struct s_binary_output_buffer{
  long long insn_count;
  uint32_t pc;
  uint32_t insn;
  char exception;
  uint32_t regs[32];
  uint32_t sr;
  uint32_t epcr0; 
  uint32_t eear0; 
  uint32_t eser0;
} __attribute__((__packed__));

struct s_binary_output_buffer_sans_regs{
  long long insn_count;
  uint32_t pc;
  uint32_t insn;
  char exception;
} __attribute__((__packed__));

void printUsage()
{
  cerr << endl << "Error: No input file specified." << endl;
  cerr << endl;
  cerr << "Usage: binlog2readable <options> <file>" << endl;
  cerr << endl;
  cerr << "Convert binary formatted ORPSoC execution log <file> to human readable format" << endl;

  cerr << "Options:" << endl;
  cerr << "  -o <file>\tOutput file, if not specified stdout is used" << endl;
  cerr << "  --skip <num>\tSkip <num> instructions from the front of the log" << endl;
  cerr << "  --last <num>\tPrint the last <num> instructions from the end of the log" << endl;
  cerr << endl;

}

int main(int argc, char **argv)
{
  ifstream inFile;
  ofstream outFile; /* Anyone know how to make these things into stdouts?? */
  unsigned long long filesize;
  int using_std_out = 1; /* Default out is to stdout */
  char reg_vals_included;
  int skip_insns = 0;
  int skip_insns_from_end = -1;
  struct s_binary_output_buffer buf;
  struct s_binary_output_buffer_sans_regs buf_sans_regs;
  
  if (argc < 2)
    {      
      printUsage();
      return 1;
    }

  for(int i=1; i < argc; i++)
    {
      if ((strcmp(argv[i], "-s")==0) ||
	  (strcmp(argv[i], "--skip")==0))
	{	      
	  if (i+1 < argc)
	    if(argv[i+1][0] != '-')
	      {
		skip_insns = atoi(argv[i+1]);
		skip_insns_from_end = 0;
		i++;
	      }
	}
      else if (strcmp(argv[i], "--last")==0)
	{
	  if (i+1 < argc)
	    if(argv[i+1][0] != '-')
	      {
		skip_insns = atoi(argv[i+1]);
		skip_insns_from_end = 1;
		i++;
	      }
	}
      else if (strcmp(argv[i], "-o")==0)
	{
	  if (i+1 < argc)
	    if(argv[i+1][0] != '-')
	      {
		/* Were given a file to output to */
		outFile.open(argv[i+1], ios::out);
		if (!outFile.is_open())
		  {
		    cerr << "Error: unable to open file " << argv[2] << " for writing." << endl;
		    if (inFile.is_open()) inFile.close();
		    return 1;
		    
		  }
		
		using_std_out = 0;
		
	      }
	}
      else
	{
	  /* Input file */
	  if (!inFile.is_open())
	    {
	      inFile.open(argv[i], ios::in | ios::binary);
	      if (!inFile.is_open())
		{
		  cerr << "Error: unable to open file " << argv[1] << " for reading." << endl;
		  return 1;
		}
	    }
	  
	}
    }
  
  
  /* First byte contains whether we've got register values included or 
  just executed instruction, pc and number*/
  inFile.seekg(0); /* Position getpointer to start of file*/
  inFile.read(&reg_vals_included, 1);

  if (skip_insns)
    {
      /* Position the file pointer to the right place before printing out */
      if (skip_insns_from_end == -1)
	goto close_exit;
      if (skip_insns_from_end)
	{
	  
	  // go to end
	  inFile.seekg(0,ios::end);
	  filesize = inFile.tellg();
	  inFile.seekg(0);
	  //cout << "filesize: " << filesize << endl;
	  // seek backwards
	  if (reg_vals_included)
	    for (int i=0;i<skip_insns;i++)
	      inFile.seekg(filesize-skip_insns*sizeof(struct s_binary_output_buffer));
	  else
	    for (int i=0;i<skip_insns;i++)
	      inFile.seekg(filesize-(skip_insns*sizeof(struct s_binary_output_buffer_sans_regs)));

	}
      else
	{
	  // skip from start
	  inFile.seekg(1);
	  if (reg_vals_included)
	    inFile.seekg(skip_insns*sizeof(struct s_binary_output_buffer),ios::cur);
	  else
	    inFile.seekg((skip_insns*sizeof(struct s_binary_output_buffer_sans_regs)),ios::cur);
	
	}	  
    }
  else
    inFile.seekg(1);
  
  //cout << "starting at: " << inFile.tellg() << endl;
  
  if (reg_vals_included)
    {
      if (using_std_out)
	{
	  while (1)
	    {
	      
	      inFile.read((char*)&buf, sizeof(struct s_binary_output_buffer));
	      
	      if(!inFile.eof())
		{
		  cout << "\nEXECUTED("<< std::setfill(' ') << std::setw(11) << dec << buf.insn_count << "): " << std::setfill('0') << hex << std::setw(8) << buf.pc << ":  " << hex << std::setw(8) << buf.insn;
		  if(buf.exception) cout << "  (exception)";
		  cout << endl;
		  
		  // Print general purpose register contents
		  for (int i=0; i<32; i++)
		    {
		      if ((i%4 == 0)&&(i>0)) cout << endl;
		      cout << std::setfill('0');
		      cout << "GPR" << dec << std::setw(2) << i << ": " <<  hex << std::setw(8) << buf.regs[i] << "  ";		
		    }
		  cout << endl;	      
		  cout << "SR   : " <<  hex << std::setw(8) << buf.sr << "  ";
		  cout << "EPCR0: " <<  hex << std::setw(8) << buf.epcr0 << "  ";
		  cout << "EEAR0: " <<  hex << std::setw(8) << buf.eear0 << "  ";	
		  cout << "ESR0 : " <<  hex << std::setw(8) << buf.eser0 << endl;
		}
	      else
		break;
      
	    }
	}
      else
	{
	  while(1)
	    {
	      /* Outputting to a file */
	      inFile.read((char*)&buf, sizeof(struct s_binary_output_buffer));
	      //inFile.seekg(sizeof(struct s_binary_output_buffer), ios::cur);
	      if(!inFile.eof())
		{
		  outFile << "\nEXECUTED("<< std::setfill(' ') << std::setw(11) << dec << buf.insn_count << "): " << std::setfill('0') << hex << std::setw(8) << buf .pc << ":  " << hex << std::setw(8) << buf.insn;
		  if(buf.exception) outFile << "  (exception)";
		  outFile << endl;
		  
		  // Print general purpose register contents
		  for (int i=0; i<32; i++)
		    {
		      if ((i%4 == 0)&&(i>0)) outFile << endl;
		      outFile << std::setfill('0');
		      outFile << "GPR" << dec << std::setw(2) << i << ": " <<  hex << std::setw(8) << buf.regs[i] << "  ";		
		    }
		  outFile << endl;	      
		  outFile << "SR   : " <<  hex << std::setw(8) << buf.sr << "  ";
		  outFile << "EPCR0: " <<  hex << std::setw(8) << buf.epcr0 << "  ";
		  outFile << "EEAR0: " <<  hex << std::setw(8) << buf.eear0 << "  ";
		  outFile << "ESR0 : " <<  hex << std::setw(8) << buf.eser0 << endl;
		}
	      else
		break;
	    
	    }
	}
    }
      else /* No regs in data */
    {
      if (using_std_out)
	{
	  while(1)
	    {
	      inFile.read((char*)&buf_sans_regs, sizeof(struct s_binary_output_buffer_sans_regs));
	      if (!inFile.eof())
		{
		  cout << "\nEXECUTED("<< std::setfill(' ') << std::setw(11) << dec << buf_sans_regs.insn_count << "): " << std::setfill('0') << hex << std::setw(8) << buf_sans_regs.pc << ":  " << hex << std::setw(8) << buf_sans_regs.insn;
		  if(buf_sans_regs.exception) cout << "  (exception)";
		  cout << endl;
		}
	      else
		break;
	    }
	  cout << endl;
	}
      else
	{
	  /* Outputting to a file */
	  while(!inFile.eof())
	    {
	      inFile.read((char*)&buf_sans_regs, sizeof(struct s_binary_output_buffer_sans_regs));
	      //inFile.seekg(sizeof(struct s_binary_output_buffer_sans_regs), ios::cur);
	      outFile << "\nEXECUTED("<< std::setfill(' ') << std::setw(11) << dec << buf_sans_regs.insn_count << "): " << std::setfill('0') << hex << std::setw(8) << buf_sans_regs.pc << ":  " << hex << std::setw(8) << buf_sans_regs.insn;
	      if(buf_sans_regs.exception) outFile << "  (exception)" << endl;
	      outFile << endl;
	      
	    }
	  outFile << endl;
	}
    }

 close_exit:
  inFile.close();
  if (!using_std_out) outFile.close();

}
