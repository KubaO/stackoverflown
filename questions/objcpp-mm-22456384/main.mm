//main.mm
#import <Foundation/NSUserNotification.h>
#import <AppKit/NSApplication.h>
#include <QCoreApplication>

int main(int argc, char ** argv)
{
   QCoreApplication a(argc, argv);
   NSApplication * app = [NSApplication sharedApplication];
   return 0;
}

