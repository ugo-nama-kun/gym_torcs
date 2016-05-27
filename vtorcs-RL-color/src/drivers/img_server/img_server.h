
#ifdef _WIN32
  typedef sockaddr_in tSockAddrIn;
  typedef int socklen_t;
  #define CLOSE(x) closesocket(x)
  #define INVALID(x) x == INVALID_SOCKET
#else
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <unistd.h>
  typedef int SOCKET;
  typedef struct sockaddr_in tSockAddrIn;
  #define CLOSE(x) close(x)
  #define INVALID(x) x < 0
#endif


#define NBBOTS 3

static char * botname[NBBOTS] = { "img1_Red", "img2_Green", "img3_Blue" };


static const int BUFSIZE = 20;

static Driver * driver[NBBOTS];


#define UDP_ID "SCR"
#define UDP_DEFAULT_TIMEOUT 10000
#define UDP_MSGLEN 64*64+1000
static int UDP_TIMEOUT = UDP_DEFAULT_TIMEOUT;
static int listenSocket[NBBOTS];
socklen_t clientAddressLength[NBBOTS];
tSockAddrIn clientAddress[NBBOTS], serverAddress[NBBOTS];

#define RACE_RESTART 1




// Maintain control in case of timeout of UDP client
static tdble oldAccel[NBBOTS];
static tdble oldBrake[NBBOTS];
static tdble oldSteer[NBBOTS];
static tdble oldClutch[NBBOTS];
static tdble prevDist[NBBOTS];
static tdble distRaced[NBBOTS];
static int oldFocus[NBBOTS];//ML
static int oldGear[NBBOTS];


static tTrack	* curTrack;
static int RESTARTING[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newrace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void endrace(int index, tCarElt *car, tSituation *s);
static void shutdown(int index);
static int  InitFuncPt(int index, void *pt);
