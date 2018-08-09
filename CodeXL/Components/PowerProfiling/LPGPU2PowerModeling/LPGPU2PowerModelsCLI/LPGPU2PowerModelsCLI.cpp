
// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>

// #include "lpgpuPowerModelsConfigManager.h"
// #include "lpgpuPowerModelsFactory.h"
// #include "lpgpuPowerModel.h"

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsConfigManager.h>


int main(int argc, char* argv[])
{    
    GT_UNREFERENCED_PARAMETER(argc);
    GT_UNREFERENCED_PARAMETER(argv);

    printf("Running %s ... \n", argv[0]);

    gtString deviceName;
    deviceName = deviceName.fromUtf8String("Nexus Player");
    auto pwDesciptors =  lpgpuPowerModelsConfigManager::instance().getSupportedPowerModels(deviceName);


    for (auto& pwDesciptor : pwDesciptors) {

        printf("Model uuid %s  \n", pwDesciptor.modelUuid.asUTF8CharArray());
        printf("Model name %s  \n", pwDesciptor.name.asUTF8CharArray());
        printf("Model description %s  \n", pwDesciptor.description.asUTF8CharArray());
        printf("Model long description %s  \n", pwDesciptor.longDescription.asUTF8CharArray());

        for(const auto& reqCounter : pwDesciptor.requiredCounters) {
            printf("  Required counter  id %u name %s modelMappedName %s \n", reqCounter.counterId, reqCounter.name.asUTF8CharArray(), reqCounter.modelMappedName.asUTF8CharArray());
        }

    }


    //lpgpuPowerModelsFactory::instance().createPowerModel(pwDesciptors[0], )


}


// int main(int argc, char* argv[])
// {
    
//     printf("Running %s ... \n", argv[0]);

//     lpgpu2::PowerModels powerModelsInstance{};
//     powerModelsInstance.init( lpgpuPowerModelsConfigManager::instance().getConfigFilePath().asString() );

//     auto& supported = powerModelsInstance.GetElements();

//     for(const auto& supportedDevices : supported.GetDevices())
//     {
//        printf("Supported Device name %s  \n", supportedDevices.GetName().asUTF8CharArray()); 
//        printf("Supported Device Id %u \n", supportedDevices.GetId());
//        printf("Supported Device description %s \n", supportedDevices.GetDescription().asUTF8CharArray());
//        //printf("Supported Device description %s ... \n", support.GetCounterElemsVec());

//        for (const auto& availableCounters : supportedDevices.GetCounterElemsVec())
//        {
//            printf("Available counter id %u \n", availableCounters.GetId()); 
//            printf("Available counter name %s \n", availableCounters.GetName().asUTF8CharArray());
//            printf("Available counter description %s \n", availableCounters.GetDescription().asUTF8CharArray());
//            printf("Available counter model mapped name %s \n", availableCounters.GetModelMappedName().asUTF8CharArray());
//        }
//     }

//     for(const auto& powerModel : powerModelsInstance.GetElements().GetPwrModels())
//     {
//         //printf("Power model name %s \n", powerModel.GetDevices());
//         printf("Power model UUId %s \n", powerModel.GetUUID().asUTF8CharArray()); 
//         printf("Power model name %s \n", powerModel.GetName().asUTF8CharArray());
//         printf("Power model description %s \n", powerModel.GetDescription().asUTF8CharArray()); 
//         for(const auto& devices : powerModel.GetDevices())
//         {
//             printf("Supported Device id %u \n", devices.GetId());
            
//             for(const auto& reqCounters : devices.GetRequiredCounters())
//             {
//                printf("Required counter  id %u \n", reqCounters); 
//             }
//         }

//         for(const auto& modelParameter : powerModel.GetParameterElemsVec()) 
//         {
//             printf("Parameter name %s \n", modelParameter.GetName().asUTF8CharArray());
//             printf("Parameter type %s \n", modelParameter.GetType().asUTF8CharArray()); 
//             printf("Parameter value %s \n", modelParameter.GetValue().asUTF8CharArray());
//         }
//     }
    

//     //auto deviceName = supported.GetDevices()[1].GetName().asUTF8CharArray();


//     exit(0);
// }