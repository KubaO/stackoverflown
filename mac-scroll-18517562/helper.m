#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>

void disableMomentumScroll(void)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObject:@"NO" forKey:@"AppleMomentumScrollSupported"];
    [defaults registerDefaults:appDefaults];
}

#if 0

#include <stdio.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
/* didn't really sort the headers out, but it compiles and runs here */

int main(int argc, char **argv) {
   const uint32_t accel = 0x80000000; /* signed 2's complement (probably) (using unsigned for setting-bits-by-hand reasons) 16.16 fixed point; normally non-negative, otherwise all acceleration and scaling will be disabled */
   io_connect_t handle = NXOpenEventStatus(); /* NXEventHandle and io_connect_t are the same thing */
   if(handle) {
      if(IOHIDSetParameter(handle, CFSTR(kIOHIDMouseAccelerationType), &accel, sizeof accel) != KERN_SUCCESS) {
         printf("Failed to set mouse accel\n");
      }
      if(IOHIDSetParameter(handle, CFSTR(kIOHIDTrackpadAccelerationType), &accel, sizeof accel) != KERN_SUCCESS) {
         printf("Failed to set trackpad accel\n");
      }
      NXCloseEventStatus(handle);
   } else {
      printf("No handle\n");
   }
   return 0;
}

#endif
