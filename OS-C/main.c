//#include "2014-01-23/parseLine.h"
//#include "2014-01-23/ex2.h"
//#include "2013-09-13/compareDir.h"
//#include "2013-07-18/2013-07-18.h"
//#include "2013-06-20/printINode.h"
//#include "2013-06-20/sameContent.h"
//#include "2013-05-29/testEventFD.h"
//#include "2013-01-25/ex1.h"
//#include "2012-07-17/concurrentCopies.h"
//#include "2012-06-20/ex1.h"
//#include "2012-06-20/ex2.h"
//#include "2012-05-30/miniMakeFile.h"
//#include "2012-01-20/optimizeSparseFile.h"
//#include "2011-09-12/compareDirectories.h"
//#include "2011-09-12/ex2.h"
//#include "2011-07-22/runAll.h"
//#include "2011-06-22/signalHandling.h"
//#include "2011-05-30/client.h"
//#include "2011-05-30/server.h"
//#include "2011-02-15/reActivate.h"
//#include "2011-01-19/parseAndRedirect.h"
//#include "2010-09-13/writer.h"
//#include "2010-09-13/reader.h"
//#include "2006-01-24/sendSignal.h"
//#include "2006-02-15/outFIFO.h"
#include "2005-01-25/censorship.h"
//#include "2005-02-10/ex1.h"
//#include "2005-06-27/linearAccelerator.h"

#include <stdio.h>
#include <stdlib.h>

// Entry point
int main(int argc, char *argv[])
{
	int c = 4;
	char *v[] = {"function", "Alliance", "cat", "publicSpeech"};

	run(c, v);

	printf("Done!");
	exit(EXIT_SUCCESS);
}
