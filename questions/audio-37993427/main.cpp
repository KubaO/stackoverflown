// https://github.com/KubaO/stackoverflown/tree/master/questions/audio-37993427
#include <QtMultimedia>
#include <array>

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   auto device = QAudioDeviceInfo::defaultOutputDevice();
   QAudioFormat format;
   format.setSampleRate(44100);
   format.setChannelCount(2);
   format.setSampleSize(16);
   format.setCodec("audio/pcm");
   format.setByteOrder(QAudioFormat::LittleEndian);
   format.setSampleType(QAudioFormat::SignedInt);
   if (!device.isFormatSupported(format))
      qFatal("Default format not supported");
   QAudioOutput audioOutput{device, format};
   auto output = audioOutput.start();
   qDebug() << audioOutput.state();
   std::array<char, 32768> buffer;
   buffer.fill(0);

   auto write = [&]{
      qDebug() << "notify";
      auto periodSize = audioOutput.periodSize();
      auto chunks = audioOutput.bytesFree() / periodSize;
      for (int i = 0; i < chunks; ++i) {
         if (periodSize && output) {
            auto len = output->write(buffer.data(), periodSize);
            if (len != periodSize)
               break;
         }
      }
   };

   audioOutput.setNotifyInterval(20);
   QObject::connect(&audioOutput, &QAudioOutput::notify, write);
   write();
   return app.exec();
}
