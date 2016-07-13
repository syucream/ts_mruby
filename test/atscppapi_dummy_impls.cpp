#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/RemapPlugin.h>
#include <atscppapi/Transaction.h>

/*
 * Emply implementations not to link libatscppapi.so
 *
 * except mocking to write unit tests related to atscppapi's behaviors
 *
 */
namespace atscppapi {

RemapPlugin::RemapPlugin(void**) {}

void Transaction::resume() {}

void GlobalPlugin::registerHook(Plugin::HookType) {}
GlobalPlugin::GlobalPlugin(bool ignore_internal_transactions) {};
GlobalPlugin::~GlobalPlugin() {}

void RegisterGlobalPlugin(std::string name, std::string vendor, std::string email) {}

}; // namespace atscppapi
