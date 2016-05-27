
/* Dump to disk in '.rgb' format */

void ssgaScreenDump ( char *filename,
                      int width, int height,
                      int frontBuffer = TRUE ) ;

/* Dump to a memory buffer - three bytes per pixel */

unsigned char *ssgaScreenDump ( int width, int height,
                                int frontBuffer = TRUE ) ;



