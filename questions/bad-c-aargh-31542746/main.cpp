#define NEW

#ifndef OLD
// New Code

#include <QString>
#include <QDebug>
#include <functional>

const QString TAG = QStringLiteral("HRAD_CONTROLLER:");
const QString HRAD_API_VERSION = QStringLiteral("2");
const QString MANUFACTURER_KEY = QStringLiteral("abcde12345");

struct RegisterDeviceParams {
   QString brand;
   QString device;
   QString manufacturer;
   QString model;
   QString serial;
   QString ibd;
   QString ibkey;
   QString version;
   QString track;
};

struct GetStationModelParams {
   int stationID;
   QString ibd;
   QString ibkey;
   QString version;
   QString track;
};

struct GetStationModelReponse {};

class HRAD {
public:
   void registerDevice(const RegisterDeviceParams &,
                       const std::function<void(const QString & ibd)> & cb
                       = std::function<void(const QString&)>()) {
      cb("some ibd");
   }

   void getStationModel(const GetStationModelParams &,
                        const std::function<void(const GetStationModelReponse &)> & cb
                        = std::function<void(const GetStationModelReponse&)>()) {
      GetStationModelReponse response;
      cb(response);
   }

   QString getId() { return "some id"; }
   int currentPublicStationID;
   HRAD() : currentPublicStationID(1) {}
};

Q_GLOBAL_STATIC(HRAD, hbr) // no semicolon needed

void getStationModel(int id){
   GetStationModelParams params;
   params.stationID = id;
   params.ibd = hbr->getId();
   params.ibkey = "abdef";
   params.version = "2";
   params.track = "false";

   hbr->getStationModel(params, [](const GetStationModelReponse&){
      qDebug() << "got station model";
   });
}

void registerDevice(){
   RegisterDeviceParams params;
   params.brand = "brand";
   params.device = "TI-J6";
   params.manufacturer = "manuf";
   params.model = "EVK";
   params.ibkey = MANUFACTURER_KEY;
   params.serial = "abcde12345";
   params.version = HRAD_API_VERSION;
   params.track = "true";

   hbr->registerDevice(params, [](const QString & ibd){
      qDebug() << TAG << "Device Registered: ibd = " << ibd;
      getStationModel(hbr->currentPublicStationID);
   });
}

int main()
{
   registerDevice();
   return 0;
}

#endif

#ifdef OLD
// Old Code

#include <QString>
#include <QDebug>

typedef void (*registerDeviceCB_t)(QString);
typedef void (*getStationModelCB_t)();
typedef struct getStationModelResponse {} GET_STATION_MODEL_RESPONSE_t;
void deviceRegisteredCB(QString);
void stationModelAvailableCB() {}

typedef struct registerDeviceParams REGISTER_DEVICE_PARAMS_t;
struct HRAD {
   void registerDevice(REGISTER_DEVICE_PARAMS_t*);
   QString getId() { return QString(); }
};

GET_STATION_MODEL_RESPONSE_t g_StationModel;
int currentPublicStationID = 1;

void registerDevice();
void getStationModel(int);
int main()
{
   registerDevice();
   getStationModel(0);
   return 0;
}

// Your code verbatim beyond this point

#define TAG "HRAD_CONTROLLER:\0"
#define HRAD_API_VERSION        "2"
#define MANUFACTURER_KEY        "abcde12345"

typedef struct registerDeviceParams{
   QString brand;
   QString device;
   QString manufacturer;
   QString model;
   QString serial;
   QString ibd;
   QString ibkey;
   QString version;
   QString track;

   registerDeviceCB_t cbFn;
}REGISTER_DEVICE_PARAMS_t;

typedef struct getStationModelParams{
   int stationID;
   QString ibd;
   QString ibkey;
   QString version;
   QString track;

   getStationModelCB_t cbFn;

   GET_STATION_MODEL_RESPONSE_t* resp;
}GET_STATION_MODEL_PARAMS_t;

HRAD *hbr = new HRAD();

void registerDevice(){
   REGISTER_DEVICE_PARAMS_t params;
   params.brand = "brand";
   params.device = "TI-J6";
   params.manufacturer = "manuf";
   params.model = "EVK";
   params.ibkey = MANUFACTURER_KEY;
   params.serial = "abcde12345";
   params.version = HRAD_API_VERSION;
   params.track = "true";
   params.cbFn = deviceRegisteredCB;

   hbr->registerDevice(&params);
}

void getStationModel(int id){
   GET_STATION_MODEL_PARAMS_t params;
   params.stationID = id;
   params.ibd = hbr->getId();
   params.ibkey = "abdef";
   params.version = "2";
   params.track = "false";  //SIGSEGV segmantation fault here
   params.cbFn = stationModelAvailableCB;

   params.resp = &g_StationModel;
}

void deviceRegisteredCB(QString ibd){
   qDebug() << TAG << "Device Registered: ibd = " << ibd;

   getStationModel(currentPublicStationID);
}

// End of verbatim code

void HRAD::registerDevice(REGISTER_DEVICE_PARAMS_t* params) {
   params->cbFn("some ibd");
}

#endif
