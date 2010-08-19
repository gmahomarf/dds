/**
 *
 * @file dds.h
 * @brief libDDS Header file
 *
 * This is the main header file for libDDS
 *
 * @author Gazy Mahomar
 *
 */

#ifndef _DDS_H_
#define _DDS_H_

/*
 *  Values and descriptions obtained
 *  from DDS Specification document.
 */

/**
 * @brief DDS Message Type Codes.
 *
 * Type codes used to determine the type of message being sent.
 *
 */
typedef enum typecode_e {
    IdHello = 'a', /**< Client unauthenticated connect request.
                     Server accept/reject response.*/
    IdGoodbye = 'b', /**< Client sends terminal handshake message before
                       disconnect. Echoed back to client by server.*/
    IdStatus = 'c', /**< Client requests the servers status.
                      Server response is a block of XML.*/
    IdStop = 'e', /**< Used to abort data retrievals that may take a long time
                    to time-out. This command is essentially a NOOP. Server
                    echoes this message as a response to the abort.*/
    IdDcp = 'f', /**< Client request for next DCP message. Server response
                   containing error or DCP message.*/
    IdCriteria = 'g', /**< Client reads or writes search criteria on the server.
                        The same type-code used for bidirectional transfer.*/
    IdGetOutages = 'h', /**< Client requests recent outages on the server.
                          Server response contains the outage information in
                          formatted text.*/
    IdPutNetlist = 'j', /**< Client uploads a network list to the server. Server
                            accept/reject response.*/
    IdGetNetlist = 'k', /**< Client requests download of a network list from the
                            server. Server response contains error or the
                            network list.*/
    IdAssertOutages = 'l', /**< Client sends one or more outages as formatted
                            text strings. Server accept/reject response.*/
    IdAuthHello = 'm', /**< Authenticated connect message containing hash of
                         password. Server accept/reject response.*/
    IdDcpBlock = 'n', /**< Client request for next block of DCP messages. Server
                        response containing multiple DCP messages in one
                        DdsMessage*/
    IdEvents = 'o', /**< Client requests recent events on the server Server
                      response contains events in formatted text.*/
    IdRetConfig = 'p', /**< Client requests a named configuration file on the
                         server. Server response contains the file or error
                         message.*/
    IdInstConfig = 'q', /**< Client sends configuration data to the server.
                          Server accept/reject response.*/
    IdDcpBlockExt = 'r', /**< Client requests the next block of DCP messages in
                            exteneded format. Server response contains DCP
                            messages or error message.*/
    IdUser = 'u' /**< User administration commands: Add, Modify, Delete */
} TYPECODE;

/**
 *
 * @brief DDS ServerCodes
 *
 * Codes returned by the server after a message is sent.
 *
 * NOTE: A ServerCode return value of -1 indicates an internal libDDS error. All
 * internal error messages will be returned in the resp parameter of the DDS
 * function that returned the error.
 *
 */
typedef enum servercode_e {
    DSUCCESS = 0, /**< Success!*/
    DNOFLAG = 1, /**< Could not find start of message flag.*/
    DDUMMY = 2, /**< Message found (and loaded) but it's a dummy.*/
    DLONGLIST = 3, /**< Network list was too long to upload.*/
    DARCERROR = 4, /**< Error reading archive file.*/
    DNOCONFIG = 5, /**< Cannot attach to configuration shared memory*/
    DNOSRCHSHM = 6, /**< Cannot attach to search shared memory*/
    DNODIRLOCK = 7, /**< Could not get ID of directory lock semaphore*/
    DNODIRFILE = 8, /**< Could not open message directory file*/
    DNOMSGFILE = 9, /**< Could not open message storage file*/
    DDIRSEMERR = 10, /**< Error on directory lock semaphore*/
    DMSGTIMEOUT = 11, /**< Timeout waiting for new messages*/
    DNONETLIST = 12, /**< Could not open network list file*/
    DNOSRCHCRIT = 13, /**< Could not open search criteria file*/
    DBADSINCE = 14, /**< Bad since time in search criteria file*/
    DBADUNTIL = 15, /**< Bad until time in search criteria file*/
    DBADNLIST = 16, /**< Bad network list in search criteria file*/
    DBADADDR = 17, /**< Bad DCP address in search criteria file*/
    DBADEMAIL = 18, /**< Bad electronic mail value in search criteria file*/
    DBADRTRAN = 19, /**< Bad retransmitted value in search criteria file*/
    DNLISTXCD = 20, /**< Number of network lists exceeded*/
    DADDRXCD = 21, /**< Number of DCP addresses exceeded*/
    DNOLRGSLAST = 22, /**< Could not open last read access file*/
    DWRONGMSG = 23, /**< Message doesn't correspond with directory entry*/
    DNOMOREPROC = 24, /**< Can't attach: No more processes allowed*/
    DBADDAPSSTAT = 25, /**< Bad DAPS status specified in search criteria.*/
    DBADTIMEOUT = 26, /**< Bad TIMEOUT value in search criteria file.*/
    DCANTIOCTL = 27, /**< Cannot ioctl() the open serial port.*/
    DUNTILDRS = 28, /**< Specified 'until' time reached*/
    DBADCHANNEL = 29, /**< Bad GOES channel number specified in search
                        criteria*/
    DCANTOPENSER = 30, /**< Can't open specified serial port.*/
    DBADDCPNAME = 31, /**< Unrecognized DCP name in search criteria*/
    DNONAMELIST = 32, /**< Cannot attach to name list shared memory.*/
    DIDXFILEIO = 33, /**< Index file I/O error*/
    DNOSRCHSEM = 34, /**< Cannot attach to search semaphore*/
    DUNTIL = 35, /**< Specified 'until' time reached*/
    DJAVAIF = 36, /**< Error in Java - Native Interface*/
    DNOTATTACHED = 37, /**< Not attached to LRGS native interface*/
    DBADKEYWORD = 38, /**< Bad keyword*/
    DPARSEERROR = 39, /**< Error parsing input file*/
    DNONAMELISTSEM = 40, /**< Cannot attach to name list semaphore.*/
    DBADINPUTFILE = 41, /**< Cannot open or read specified input file*/
    DARCFILEIO = 42, /**< Archive file I/O error*/
    DNOARCFILE = 43, /**< Archive file not opened*/
    DICPIOCTL = 44, /**< Error on ICP188 ioctl call*/
    DICPIOERR = 45, /**< Error on ICP188 I/O call*/
    DINVALIDUSER = 46, /**< Invalid user name*/
    DDDSAUTHFAILED = 47, /**< DDS Authentication Failure*/
    DDDSINTERNAL = 48, /**< DDS Internal Error*/
    DDDSFATAL = 49, /**< DDS Fatal internal server error*/
    DNOSUCHSOURCE = 50, /**< Search criteria specified an invalid data source.*/
    DALREADDYATTACHED = 51, /**< This user is already connected and multiple
                                 connections by the same user have been
                                 disallowed. */
    DNOSUCHFILE = 52 /**< The client has requested a file that doesn’t exist.*/
} SERVERCODE;

/**
 * @brief DDS Response Message Structure.
 *
 * Structure used to store a server response message.
 *
 */
 typedef struct dds_response {
    char sync[5];/**< Response message header sync (always "FAF0") */
    TYPECODE typeCode;/**< Response message Type */
    SERVERCODE serverCode;/**< Response message Server Code */
    unsigned int systemCode;/**< Error message System Code (0, except on error) */
    unsigned long msgLen;/**< Response message length*/
    unsigned char* body;/**< Response message body */
} DDSRESPONSE;

/**
 * @brief Send a DDS Message.
 *
 * Function used to send messages to the server.
 *
 * @param tc TYPECODE of the message to be sent.
 * @param msg Message to be sent to the server.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSSendMessage(TYPECODE tc, char* msg, DDSRESPONSE* resp);

/**
 * @brief DDS Connect by assertion.
 *
 * Function used to connect to the server by assertion i.e. only using a user
 * name, without a password.
 *
 * @param server Host name or IP address of server.
 * @param port Port number to connect to.
 * @param user User name to assert as.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSConnectAssert(char* server, char* port, char* user,
                            DDSRESPONSE* resp);

/**
 * @brief DDS Connect by authentication.
 *
 * Function used to connect to the server by authentication i.e. using a user
 * name and a password.
 *
 * @param server Host name or IP address of server.
 * @param port Port number to connect to.
 * @param user User name used to authenticate.
 * @param pass Password used to authenticate user.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSConnectAuth(char* server, unsigned short port, char* user,
                          char* pass, DDSRESPONSE* resp);

/**
 * @brief DDS Disconnect.
 *
 * Function used to disconnect from the server.
 *
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSDisconnect(DDSRESPONSE* resp);

/**
 * @brief Send search criteria.
 *
 * Function used to send search criteria to the server.
 *
 * @param sc Path (relative or absolute) to search criteria file.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSSendSearchCriteria(char* sc, DDSRESPONSE* resp);

/**
 * @brief Send network list.
 *
 * Function used to send a network list to the server.
 *
 * @param nl Path (relative or absolute) to network list file.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSSendNetworkList(char* nl, DDSRESPONSE* resp);

/**
 * @brief Get network list.
 *
 * Function used to fetch a network list from the server.
 *
 * @param nl Network list name.
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSGetNetworkList(char* nl, DDSRESPONSE *resp);

/**
 * @brief Get a single DCP Message.
 *
 * Function used to fetch a single DCP Message from the server.
 *
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSGetDcpMessage(DDSRESPONSE* resp);

/**
 * @brief Get multiple DCP Messages.
 *
 * Function used to fetch a DCP Message block from the server.
 *
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSGetMultipleDcpMessages(DDSRESPONSE* resp);

/**
 * @brief Get extended multiple DCP Messages.
 *
 * Function used to fetch an extended DCP Message block from the server.
 * If successful, the Message Block will be an XML block with the following
 * format:
 *
 * @verbatim
  <MsgBlock>
    <DcpMsg flags=0xnnnn>
        <BinaryMsg> BASE64(DOMSAT Header and msg data) </BinaryMsg>
        [<CarrierStart>YYYY/DDD HH:MM:SS.mmm</carrierStart>]
        [<CarrierStop>YYYY/DDD HH:MM:SS.mmm</carrierStop>]
        [<DomsatTime>YYYY/DDD HH:MM:SS.mmm</domsatTime>]
        [<DomsatSeq>NNNNN</domsatSeq>]
        [<Baud>NNNN</baud>]
    </DcpMsg>
    ...additional DcpMsg blocks here
  </MsgBlock>
  @endverbatim
 *
 * @param resp Pointer to structure where server response will be stored.
 * @return The ServerCode returned by the server. In case of an error, -1
 *         will be returned, with an error description in the 'body' member
 *         of resp.
 *
 */
SERVERCODE DDSGetMultipleDcpMessagesExtended(DDSRESPONSE* resp);

#endif /* _DDS_H_ */
