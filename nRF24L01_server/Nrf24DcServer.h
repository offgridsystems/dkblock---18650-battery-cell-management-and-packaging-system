
#ifndef NRF24_DC_SERVER_H
#define NRF24_DC_SERVER_H
#include <RF24.h>
//#include <RF24/RF24.h>
#include "dc_global.h"

#define DC_MAX_CLIENT_NUMBER 300
#define DC_MAX_SIZE_OF_DATA_FOR_SENDING 4
#define DC_NUMBER_OF_BROADCAST_REQUESTS 3

#define DC_DEFAULT_LOOKUP_TIMEOUT 35000
#define DC_DEFAULT_READING_TIMEOUT 4
#define DC_REPS_COUNT 20

typedef RF24 rfDriver;

class Nrf24DcServer
{
  public:
    Nrf24DcServer(rfDriver& drv);

    // Initializes object and driver
    bool init();

    // Thsi function id used for seeking clients
    // returns number of found clients
    // @parameter timeout - timeout for sekking in miliSec.
    //     approximate time for looking for 
    //     300 clients is 4000mSec
    // @note:
    // this function clears list of handled device
    // and store ids of found clients
    uint16_t lookForClient(int timeout = DC_DEFAULT_LOOKUP_TIMEOUT);


    // This function starts communiction session
    // for receiving data from handled clients
    // and sending them  prepared data
    // by function <putSendedData>
    // returns number of handled client.
    // for reading received data 
    // use function <receivedDataByIndex> 
    //           or <receivedDataById>  
    int16_t startSession();

    // Set number of channel for session communication
    void setWorkChannel(uint8_t ch);

    // Set number of channel for bradcast messages
    void setBroadcastChannel(uint8_t ch);

    // Returns number of work channel
    uint8_t workChannel();

    // Returns number of bradcast channel
    uint8_t broadcastChannel();

    // Set network address
    // @note: address have to be 3 bytes (0xFFFFFF is maximmum) 
    void setNetworkAddr(uint64_t nAddr);

    //Returns network address.
	uint64_t networkAddr();

    void setRFDataRate(const rf24_datarate_e speed);
    void setRF_PA_Level(uint8_t level);

    // Turn on broadcast mode.
    // This means that boradcast channel and NoACK mode will be setted
    bool setBroadcastMode();

    // Turn on single client mode on.
    // This means that work channel and AuroACK mode will be setted
    // @parameter clientAddr - address of client which will communicate with server
    bool setSingleClientMode(uint64_t clientAddr);
    
    // Returns number of handled clients added by addClient() or addClientByRange()
    int handledClientsCount();

    // Adds client which server will handle
    // @parametr clientId - id of client
    bool addClient(int16_t clientId);

    // Adds clients which server will handle by range
    // @parametr from - id of first client
    //          count - number of clients
    // @Example : addClientByRange(5,4);
    // Adds 4 client with id 5, 6, 7, 8;
    bool addClientByRange(int16_t from, int16_t count);

    // Remove client from handled list,
    // and this client won't be handled by server
    // @parameter clientId - id of device
    // if clientId == -1 all clients will be deleted from handled list
    bool removeClient(int16_t clientId);

    // Remove clients from handled list,
    // and these clients won't be handled by server
    // @parameter from - first id of client
    //           count - number of clients
    // @Example : removeClientsByRange(5,4);
    // Removes 4 client with id 5, 6, 7, 8;
    bool removeClientsByRange(int16_t from, int16_t count);

    // Returns ID of client by index from internal list of handled clients
    int16_t clientIdAt(int16_t index);

    // Returns index of client by id from internal list of handled clients
    int16_t clientIndexById(int16_t id);

    /**
     * returns true if client in array for handling
     * @parameters <id> id of client
     */
    bool isDeviceHandled(int16_t id);


    void setSessionTimeout(uint16_t timeOut);
    uint16_t sessionTimeout();

    void setReadingTimeout(uint16_t timeout);
    uint16_t readingTimeout();

    void setKeepAliveTimeout(int16_t timeout);
    int16_t keepAliveTimeout() const;

    bool sendRequestForSession();
    bool sendRequestForLookup(int32_t timeout);

    bool sendStartSesionTag(uint8_t times = 1);
    bool sendStartSesionTagWithData(const uint8_t* data, const uint8_t len);

    uint8_t* receivedDataById(uint16_t id);
    uint8_t* receivedDataByIndex(uint16_t idx);
    uint8_t  lenfgthOfReceivedBufferByIndex(uint16_t idx);
    uint8_t  lenfgthOfReceivedBufferById(uint16_t id);

    // copy data from <data> to internal buffer 
    // this data will be sended during communication session
    // @parametrs:
    //    idx - index of internal buffer
    //   data - pointer to the buffer
    //    len - size of buffer
    // @note: you can get index for client by calling 
    //         deviceIndexById(id) - where id is id of client
    void putSendedData(int16_t idx, const void *data, uint8_t len);

    void testChannel();

    void sendKeepAliveMsg();

    void serverLoop();

    void setEncryption(bool flag);
    bool encryption();
    void setEcryptKeyPointer(uint8_t *pointer, uint8_t len);
    inline void encryptMsg(uint8_t *msg, uint8_t size);
    inline void decryptMsg(uint8_t *msg, uint8_t size);

    void sendEndSessionTag();


  public:
    rfDriver &driver_;
    uint64_t networkAddress_;
	uint64_t serverAddress_;
    //uint8_t broadcastChannel_;
    uint8_t workChannel_;
    //int16_t handledDevicesCount_;
    
    int16_t handledDevicesId_[DC_MAX_CLIENT_NUMBER];
    int8_t commsStatus_[DC_MAX_CLIENT_NUMBER];
    uint8_t dataBufferFromClients_[DC_MAX_CLIENT_NUMBER][DC_MAX_SIZE_OF_RF_PACKET];
    uint8_t dataBufferFromClientsSize_[DC_MAX_CLIENT_NUMBER];
    uint8_t dataBufferToClients_[DC_MAX_CLIENT_NUMBER][DC_MAX_SIZE_OF_DATA_FOR_SENDING];
    uint8_t channelsActivity_[DC_CHANNEL_COUNT];
    int16_t currnetDeviceCount_;
    
    uint8_t sendBuffer_[32];
    //uint8_t recvBuffer_[32];
    uint16_t sessionTimeout_;
    volatile uint16_t readingTimeout_;
    int16_t keepAliveTimeout_;

    bool isEncrypt_;
    uint8_t *keyPtr_;
    uint8_t keySize_;
    uint8_t receivingEndSessionMsgTimeout_;

    void prepareArrays();
    bool sendBroadcastRequestCommand(String command);
    bool isChannelBussy(uint8_t channel);
    void scanChannels();
    int16_t lookForFreeChannel();
    bool isChannelFreeRadius(int8_t channel, int8_t radius);
    void waitEndSessionTag(uint64_t leftTime = 0);
    
    inline void read(void *buf, uint8_t len);
    inline bool write(const void *buf, const uint8_t len);

};



#endif
