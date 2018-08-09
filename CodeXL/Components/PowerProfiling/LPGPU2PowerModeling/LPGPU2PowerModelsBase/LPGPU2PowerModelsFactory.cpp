
#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsFactory.h>

lpgpuPowerModelsFactory& lpgpuPowerModelsFactory::instance() 
{
    // C++11 ensures that local static variables are only initialised once even in the presence of concurrency.
    // Concerning freeing memory. No need because we want our singleton to live for the entire life of program
    // and gets destroyed on program exit.
    static lpgpuPowerModelsFactory *factory = new lpgpuPowerModelsFactory();
    return *factory;
}

std::shared_ptr<lpgpuPowerModel> lpgpuPowerModelsFactory::createPowerModel(lpgpuPowerModelDescriptor& modelDescriptor, const gtString& dbPath) 
{

    // Engines list
    static std::vector<std::shared_ptr<lpgpuPowerModel>> mPowerModels;

    gtString temp;

    // The string used in this comparison is the one that is used in configuration file to select which model to use
    if (modelDescriptor.modelUuid.isEqual("linear_model_v1_0")) {
        auto index = mPowerModels.size();
        mPowerModels.push_back(std::make_shared<lpgpuLinearPowerModel>(modelDescriptor, dbPath));
        return mPowerModels.at(index);
    }

    // *** ADD ANY ADDITIONAL Power Models HERE ***

    //else if (modelUuid == "newemodel") {
    //return new NewModel();
    //}

    // *** ADD ANY ADDITIONAL ENGINES HERE ***

    return nullptr;

}

