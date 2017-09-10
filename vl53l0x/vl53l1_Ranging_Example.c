
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <hwlib/hwlib.h>

#include "vl53l1_api.h"
#include "vl53l1_platform_init.h"

#define trace_print(level, ...) \
    VL53L1_trace_print_module_function(VL53L1_TRACE_MODULE_CORE, \
    level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)

void print_pal_error(VL53L1_Error Status){
    char buf[VL53L1_MAX_STRING_LENGTH];
    VL53L1_GetPalErrorString(Status, buf);
    printf("API Status: %i : %s\n", Status, buf);
}

void print_multiranging_data(
        int i, int j,
        VL53L1_RangingMeasurementData_t *pRangeData) {

    printf("RangingMeasurementData[%d]\n",j);

    printf("%d: Stream Count[%d]= %d\n", i, j, pRangeData->StreamCount);
    printf("%d: DmaxMilliMeter[%d]= %d\n", i, j, pRangeData->DmaxMilliMeter);
    printf("%d: SignalRateRtnMegaCps[%d]= %f\n", i, j, pRangeData->SignalRateRtnMegaCps/65536.0);
    printf("%d: AmbientRateRtnMegaCps[%d]= %f\n", i, j, pRangeData->AmbientRateRtnMegaCps/65536.0);
    printf("%d: EffectiveSpadRtnCount[%d]= %d\n", i, j, pRangeData->EffectiveSpadRtnCount);
    printf("%d: SigmaMilliMeter[%d]= %f\n", i, j, pRangeData->SigmaMilliMeter/65536.0);
    printf("%d: RangeMilliMeter[%d]= %d\n", i, j, pRangeData->RangeMilliMeter);
    printf("%d: RangeMinMilliMeter[%d]= %d\n", i, j, pRangeData->RangeMinMilliMeter);
    printf("%d: RangeMaxMilliMeter[%d]= %d\n", i, j, pRangeData->RangeMaxMilliMeter);
    printf("%d: RangeStatus[%d]= %d\n", i, j, pRangeData->RangeStatus);
}

VL53L1_Error PrintROI(VL53L1_DEV Dev)
{
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    VL53L1_RoiConfig_t RoiConfig;
    uint8_t MaxNumberOfROI;
    int i;

    Status = VL53L1_GetMaxNumberOfROI(Dev, &MaxNumberOfROI);
    printf("MaxNumberOfROI : %d\n", MaxNumberOfROI);
    Status = VL53L1_GetROI(Dev, &RoiConfig);

    if (Status == VL53L1_ERROR_NONE) {
        for (i=0;i<RoiConfig.NumberOfRoi;i++) {
            printf("ROI number = %d\n",i);
            printf("TopLeftX = %d\n",RoiConfig.UserRois[i].TopLeftX);
            printf("TopLeftY = %d\n",RoiConfig.UserRois[i].TopLeftY);
            printf("BotRightX = %d\n",RoiConfig.UserRois[i].BotRightX);
            printf("BotRightY = %d\n",RoiConfig.UserRois[i].BotRightY);
            }
        }
    return Status;
}


int  RunRangingLoop(VL53L1_DEV Dev, int  no_of_measurements) {
    int Status = VL53L1_ERROR_NONE;
    int i,j,k;
    int no_of_object_found;
    VL53L1_RangingMeasurementData_t *pRangeData;
    VL53L1_MultiRangingData_t MultiRangingData;
    VL53L1_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
    uint32_t MeasurementTimingBudgetMicroSeconds;

    if (Status == VL53L1_ERROR_NONE) {
        printf("run VL53L1_StartMeasurement\n");
        Status = VL53L1_StartMeasurement(Dev);
        }
    if( Status != VL53L1_ERROR_NONE){
        printf("fail to StartMeasurement\n");
        return -1;
        }
    if (Status == VL53L1_ERROR_NONE)
        Status = PrintROI(Dev);

    for (i = 0 ; i < no_of_measurements+1 ; i++) {
        Status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(Dev, &MeasurementTimingBudgetMicroSeconds);
        printf("MeasurementTimingBudgetMicroSeconds: %d\n", MeasurementTimingBudgetMicroSeconds);
        /* Wait for range completion */
        if (Status == VL53L1_ERROR_NONE)
            Status = VL53L1_WaitMeasurementDataReady(Dev);

        if(Status == VL53L1_ERROR_NONE) {
            Status = VL53L1_GetMultiRangingData(Dev, pMultiRangingData);

            if (Status == VL53L1_ERROR_NONE)
                VL53L1_ClearInterruptAndStartMeasurement(Dev);

            no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
            printf("Number of measurements = %d\n",i);
            printf("Number of Objects Found = %d\n",no_of_object_found);
            printf("Internal distance mode = %d\n",(int)Dev->Data.CurrentParameters.InternalDistanceMode);
            printf("New distance mode = %d\n",(int)Dev->Data.CurrentParameters.NewDistanceMode);

            if (no_of_object_found <=1)
                k = 1;
            else
                k = no_of_object_found;

            for(j=0;j<k;j++) {
                pRangeData = &(pMultiRangingData->RangeData[j]);
                print_multiranging_data(i, j, pRangeData);

                }
            printf("\n"); 
            } 
        else 
            break;

        if (Status == VL53L1_ERROR_NONE)
            Status = PrintROI(Dev);

        Status = VL53L1_WaitUs(Dev, 100000);
        }

    if (Status == VL53L1_ERROR_NONE) {
        printf("run VL53L1_StopMeasurement\n");
        Status = VL53L1_StopMeasurement(Dev);
        }

    return Status;
}


VL53L1_Error ROIExample(VL53L1_DEV Dev)
{
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    VL53L1_RoiConfig_t RoiConfig;
    uint8_t MaxNumberOfROI;

    Status = VL53L1_GetMaxNumberOfROI(Dev, &MaxNumberOfROI);
    printf("MaxNumberOfROI : %d\n", MaxNumberOfROI);

    if (Status == VL53L1_ERROR_NONE) {
        RoiConfig.NumberOfRoi = 1;
        RoiConfig.UserRois[0].TopLeftX = 2;
        RoiConfig.UserRois[0].TopLeftY = 14;
        RoiConfig.UserRois[0].BotRightX = 14;
        RoiConfig.UserRois[0].BotRightY = 2;
        Status = VL53L1_SetROI(Dev, &RoiConfig);
        }

    return Status;
}



VL53L1_Error TimingBudgetExample(VL53L1_DEV Dev)
{
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    uint32_t MeasurementTimingBudgetMicroSeconds = 12000;

    Status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(Dev, &MeasurementTimingBudgetMicroSeconds);
    printf("Timing Budget is : %d us\n", MeasurementTimingBudgetMicroSeconds);

    if (Status == VL53L1_ERROR_NONE) 
        Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, MeasurementTimingBudgetMicroSeconds + 5000);

    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(Dev, &MeasurementTimingBudgetMicroSeconds);
        printf("New Timing Budget is : %d us\n", MeasurementTimingBudgetMicroSeconds);
        }

    return Status;
}

//int main(int argc, char **argv)
int test_vl53l0x(void)
{
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    VL53L1_Dev_t                   dev;
    VL53L1_DEV                     Dev = &dev;
    VL53L1_PresetModes             PresetMode;
    VL53L1_DeviceInfo_t            DeviceInfo;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf ("VL53L1 Ranging example\n\n");
 //   SUPPRESS_UNUSED_WARNING(argc);
 //   SUPPRESS_UNUSED_WARNING(argv);

    /*
    * Initialize the platform interface
    */
    Status = VL53L1_platform_init(
                    Dev,
                    (0x29 << 1), /* EVK requires 8-bit I2C 0010 1001 -> 0101 0010 */
                    1,           /* comms_type  I2C*/
                    400);        /* comms_speed_khz - 400kHz recommended */
printf("VL53L1_platform_init completed\n");
    /*
    * Wait 2 sec for supplies to stabilize
    */

    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_WaitMs(Dev, 2000);
printf("VL53L1_WaitMs completed\n");

    /*
    *  Wait for firmware to finish booting
    */
    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_WaitDeviceBooted(Dev);
printf("VL53L1_WaitDeviceBooted completed\n");

    /*
    * Initialise Dev data structure
    */
    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_DataInit(Dev);
printf("VL53L1_DatInit completed\n");

    if(Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_GetDeviceInfo(Dev, &DeviceInfo);
        if(Status == VL53L1_ERROR_NONE) {
            printf("VL53L1_GetDeviceInfo:\n");
            printf("Device Name : %s\n", DeviceInfo.Name);
            printf("Device Type : %s\n", DeviceInfo.Type);
            printf("Device ID : %s\n", DeviceInfo.ProductId);
            printf("ProductRevisionMajor : %d\n", DeviceInfo.ProductRevisionMajor);
            printf("ProductRevisionMinor : %d\n", DeviceInfo.ProductRevisionMinor);

            if ((DeviceInfo.ProductRevisionMajor != 1) || (DeviceInfo.ProductRevisionMinor != 1)) {
                printf("Error expected cut 1.1 but found cut %d.%d\n", 
                        DeviceInfo.ProductRevisionMajor, DeviceInfo.ProductRevisionMinor);
                Status = VL53L1_ERROR_NOT_SUPPORTED;
                }
            }
        else
            printf("Error occured during VL53L1_GetDeviceInfo\n");
        print_pal_error(Status);
        }

    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_StaticInit(Dev);
printf("VL53L1_StaticInit completed\n");

    /*
    * Run reference SPAD characterisation
    */
    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_PerformRefSpadManagement(Dev);
printf("VL53L1_PerformRefSpadManagement completed\n");

    /*
    * Initialize configuration data structures for the
    * given preset mode. Does *not* apply the settings
    * to the device just initializes the data structures
    */

    if (Status == VL53L1_ERROR_NONE) {
        PresetMode = VL53L1_PRESETMODE_RANGING;
        Status = VL53L1_SetPresetMode(Dev, PresetMode);
        }
printf("VL53L1_SetPresetMode completed\n");

    if (Status == VL53L1_ERROR_NONE) 
        Status = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_LONG);
printf("VL53L1_SetDistanceMode completed\n");

    if (Status == VL53L1_ERROR_NONE) 
        Status = VL53L1_SetOutputMode(Dev, VL53L1_OUTPUTMODE_STRONGEST);
printf("VL53L1_SetOutputMode completed\n");

    /*
     * Set ROI Example before start
    *
    */
    if (Status == VL53L1_ERROR_NONE)
        Status = ROIExample(Dev);
printf("ROIExample completed\n");
    if (Status == VL53L1_ERROR_NONE)
        Status = PrintROI(Dev);
printf("PrintROI completed\n");

    /*
     * Ranging LOOP
     *
     * Run two times the Ranging loop to test start stop
    *
    */
    /* The following ranging loop will use Vl53L1_GetMultiRangingData*/
    if (Status == VL53L1_ERROR_NONE) {
        printf("*********************************************\n");
        printf("    RUN first RunRangingLoop\n");
        printf("*********************************************\n");
        Status = RunRangingLoop(Dev, 15);
        }

    if (Status == VL53L1_ERROR_NONE)
        Status = VL53L1_platform_terminate(Dev);

    print_pal_error(Status);

    return (Status);
}

