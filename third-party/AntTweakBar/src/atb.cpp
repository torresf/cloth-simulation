#include <atb.hpp>
#include <unordered_map>

namespace atb {

void GenericSet(const void* value, void* clientData) {
    ISetGetCallbacks* callbacks = (ISetGetCallbacks*) clientData;
    callbacks->set(value);
}

void GenericGet(void* value, void* clientData) {
    ISetGetCallbacks* callbacks = (ISetGetCallbacks*) clientData;
    callbacks->get(value);
}

void GenericButtonCallback(void* clientData) {
    ButtonCallback* callback = (ButtonCallback*) clientData;
    callback->callback();
}

typedef std::unordered_map<std::string, ICallbackPtr> CallbackMap;
typedef std::unordered_map<TwBar*, CallbackMap> GlobalCallbackMap;

static GlobalCallbackMap gCallbackMap;

ISetGetCallbacks* addCallbacks(TwBar* bar, const char* name,
                               SetGetCallbackPtr callbacks) {
    ISetGetCallbacks* ptr = callbacks.get();
    gCallbackMap[bar][name] = std::move(callbacks);
    return ptr;
}

ButtonCallback* addCallback(TwBar *bar, const char *name, ButtonCallback callback) {
    auto uPtr = std::unique_ptr<ButtonCallback>(new ButtonCallback(callback));
    ButtonCallback* ptr = uPtr.get();
    gCallbackMap[bar][name] = std::move(uPtr);
    return ptr;
}

void removeVar(TwBar *bar, const char *name) {
    gCallbackMap[bar].erase(name);
    TwRemoveVar(bar, name);
}

void removeAllVars(TwBar *bar) {
    gCallbackMap.erase(bar);
    TwRemoveAllVars(bar);
}

void deleteBar(TwBar* bar) {
    gCallbackMap.erase(bar);
    TwDeleteBar(bar);
}

}
