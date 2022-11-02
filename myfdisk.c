#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>


typedef struct
{
  uint8_t IsBootable;
  uint8_t Chs_ignored[3];
  uint8_t ParitionType;
  uint8_t Chs_ignored1[3];
  uint32_t lba;
  uint32_t SectorCount;
} partitioninfo;

typedef struct
{
  uint8_t PartitionType;
  char Stringtype[20];
} partitiontype;

partitiontype Partitiontypes[4] = { {7, "HPFS/NTFS/exFAT"},
{5, "Extended"},
{0x83, "Linux"},
{0x82, "Linux swap"},
};

uint8_t IsExtended = 0;
uint32_t ExtendedStart = 0;
uint8_t Headeronce = 0;

void
PrintParitionType (uint8_t Type)
{
  for (int i = 0; i < 4; i++)
    {
      if (Partitiontypes[i].PartitionType == Type)
	{
	  printf ("%2s", Partitiontypes[i].Stringtype);
	  break;
	}
    }

}

void
HandlePartition (uint8_t fd, uint8_t * buf, char *argv[], uint8_t isextended_parser)
{
	static uint8_t iterator = 0;
  (void) read (fd, buf, 512);
  if (!((buf[510] == 0x55U) && (buf[511] == 0xAAU)))
    {
      printf ("%d\n", buf[510]);
      printf ("%d\n", buf[511]);
      printf ("Table entry is not MBR!\n");
    }
  else
    {

      partitioninfo *partitioninfoptr = (partitioninfo *) & buf[446];
      if (Headeronce == 0)
	{
	  printf ("%-10s %-9s %-15s %-7s %-9s %-5s %-5s %-5s\n", "Device",
		  "Boot", "Start", "End", "Sectors", "Size", "Id", "Type");
	  Headeronce = 1;
	}

      for (int i = 0; i < 4; i++)
	{

	    if ((partitioninfoptr[i]).lba == 00)
	    {
	      continue;
	    }

		if( isextended_parser == 1  )
		{
			(partitioninfoptr[i]).lba += ExtendedStart;
		}
	    if ((partitioninfoptr[i]).ParitionType == 5)
	    {
	      IsExtended = 1;
	      ExtendedStart = (partitioninfoptr[i]).lba;
	      continue;
	    }


	  printf ("%s%d  ", argv[1], i + iterator + 1);
	  if ((partitioninfoptr[i]).IsBootable == 0x80)
	    printf ("*    ");
	  else
	    printf ("     ");
	  printf ("%10d   ", (partitioninfoptr[i]).lba);

	  printf ("%11d   ",
		  ((partitioninfoptr[i]).lba) +
		  ((partitioninfoptr[i]).SectorCount) - 1);
	  printf ("%2d   ", (partitioninfoptr[i]).SectorCount);
	  printf ("%2dG  ",
		  (((((partitioninfoptr[i]).SectorCount / 2) / 1024) /
		    1024)));
	  printf ("%2x   ", (partitioninfoptr[i]).ParitionType);
	  PrintParitionType ((partitioninfoptr[i]).ParitionType);
	  printf ("\n");
	  if(i == 3) iterator = 4;

	}

    }

}

int
main (int argc, char *argv[])
{

  uint8_t buf[512];

  if (argc == 2)
    {
      	int fd = open (argv[1], O_RDONLY);

      	if (fd == -1)
		{
	 		 printf ("File descriptor of specified Hard disk cannot be open\n");
		}
    
		else
		{
		  	do
		    {
		    	off_t offset = 0;
		    	uint8_t isextended_parser = IsExtended;
		    	IsExtended = 0;
		    	offset = lseek(fd,(off_t)ExtendedStart*512,SEEK_SET);
		    	if(offset == -1) printf("Error in file offset!\n");
		    	else
		    	{
		    		HandlePartition (fd, buf, argv,isextended_parser);
		    	}
		    }
		    while(IsExtended == 1);
		}
    }
	else
	    printf ("Incorrect input \n");

	  return 0;
}
