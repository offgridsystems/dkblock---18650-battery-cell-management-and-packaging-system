/*
How it works.

1. Server sends command for satrting communication session
for all clients on broadcast channel.

2. Clients stops to listen bradcast channel and switch to listening workChannel
3. Server gets address from internal list of handled clients
and sends request for starting session to the certain client
4. if request is received by client server receives data from client and sends data to client
else server marks this client as unhadled

5. When last client from list will handle it will start to handle unhandled clients
until session timeout

*/


#include <RF24/RF24.h>  //standard arduino library, you can install it on library manager
#include "Nrf24DcServer.h"

RF24 driver(9, 10);           // init driver D2 - CE pin, D8 CS pin
Nrf24DcServer server(driver);

#define NETWORK_ADDR 0xC7C7C7LL

int sessionTimeout = 3000;  //in milli seconds
int readingTimeout = 2;     //in milli seconds

void setup() {
    // put your setup code here, to run once:

    Serial.begin(9600);
    Serial.print("Init nrf24 driver - ");
    Serial.println(server.init());       // initialize server and driver

    driver.setAddressWidth(5);           // must be always 5
    driver.setAutoAck(false);
    driver.setRetries(1, 15);            // set 15 retries and 500uSec between retransmition in autoACK mode.
    driver.setCRCLength(RF24_CRC_8);     // number of bits of CRC, can be RF24_CRC_8 or RF24_CRC_16
    driver.enableDynamicPayloads();      // alway use this options
    driver.setDataRate(RF24_1MBPS);      // the communication speed (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS)

    server.setWorkChannel(10);           // number of frequency channel for dirrect communicatin between server and client
    server.setBroadcastChannel(120);     // number of frequency channel for receiving commands from server
    server.setNetworkAddr(NETWORK_ADDR); // Set network address, 3 bytes

    // manual adding clients ID that will be handled by server
    //server.addClient(0xc7);
    //server.addClient(0xc8);

    //server.addDeviceByRange(1, 300);


    // putSendedData(int16_t idx, const void *data, uint8_t len)
    // copy data from <data> to internal buffer 
    // this data will be sended during communication session
    // @parametrs:
    //    idx - index of internal buffer
    //   data - pointer to the buffer
    //    len - size of buffer
    // @note: you can get index for client by calling 
    //         deviceIndexById(id) - where id is id of client
    server.putSendedData(-1, "123", 4); 


}

void loop() {

    Serial.println("Start loking for clients");
    int n = server.lookForClient(4000);
    Serial.print("Found ");
    Serial.print(n);
    Serial.println(" Devices");
    for (int i = 0; i < server.handledClientsCount(); ++i)
    {
        Serial.println(server.clientIdAt(i), HEX);
    }

    return;

    Serial.println("Sending to broadcast request for data ");
    uint64_t  startSingleSessionTime = millis();

    // for starting communication sessions between server and client call server.startSession()
    // this function returns number of handled clients 
    int numberOfHandledDevices = server.startSession();
    yield();

    uint32_t finishTime = millis() - startSingleSessionTime;
    Serial.print("Session duration ");
    Serial.print(finishTime);
    Serial.println("mSec");
    Serial.print("Number of handled devices = ");
    Serial.println(numberOfHandledDevices);

    if (numberOfHandledDevices)
    {
        for (int i = 0; i < server.handledClientsCount(); ++i)
        {
            Serial.print("Device id - 0x");
            Serial.println(server.clientIdAt(i), HEX);
            Serial.println("Received data:");

            for (int j = 0; j < server.lenfgthOfReceivedBufferByIndex(i); ++j)
                Serial.print((char)*(server.receivedDataByIndex(i)+j));

            Serial.println("\n--------------------------------------");
        }
    }

    delay(5000);
    yield();
}
