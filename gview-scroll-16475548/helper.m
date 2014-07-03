//helper.m
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>

void disableMomentumScroll(void)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObject:@"NO" forKey:@"AppleMomentumScrollSupported"];
    [defaults registerDefaults:appDefaults];
}
