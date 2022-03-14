/*
 * awsconnect.h
 *
 *  Created on: 2022Äê3ÔÂ8ÈÕ
 *      Author: L1A
 */

#ifndef AWSCONNECT_H_
#define AWSCONNECT_H_

#define MAX_URI_SIZE 128
#define URI_SIZE MAX_URI_SIZE + 1


#define APPLICATION_NAME        "SSL"
#define APPLICATION_VERSION     "1.1.1.EEC.Spring2018"
#define SERVER_NAME             "audb3f5o4mwp7-ats.iot.us-west-2.amazonaws.com"
#define GOOGLE_DST_PORT         8443

#define SL_SSL_CA_CERT "/cert/rootca.der" //starfield class2 rootca (from firefox) // <-- this one works
#define SL_SSL_PRIVATE "/cert/private.der"
#define SL_SSL_CLIENT  "/cert/client.der"


//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                7    /* Current Date */
#define MONTH               3     /* Month 1-12 */
#define YEAR                2022  /* Current year */
#define HOUR                23    /* Time - hours */
#define MINUTE              41    /* Time - minutes */
#define SECOND              0     /* Time - seconds */

#define POSTHEADER "POST /things/CC3200/shadow HTTP/1.1\n\r"
#define HOSTHEADER "Host: audb3f5o4mwp7-ats.iot.us-west-2.amazonaws.com\r\n"
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"


long WlanConnect();
int set_time();
void BoardInit(void);
long InitializeAppVariables();
int tls_connect();
int connectToAccessPoint();
long httpGetTime(int);
int http_post(int, char *, int);
void getRealTime(char * rTime, char * rDate, long sec);

#endif /* AWSCONNECT_H_ */
