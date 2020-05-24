#ifndef ATB_HPP
#define ATB_HPP

#include "AntTweakBar.h"
#include <memory>
#include <iostream>
#include <functional>

#define ATB_GEN_TYPE_HELPER(TYPE, FLAG) \
    template<> \
    struct TypeHelper<TYPE> { \
        static const TwType type_flag = FLAG; \
    }

#define ATB_VAR(VAR) #VAR, VAR

namespace atb {

template<typename T>
struct TypeHelper;

ATB_GEN_TYPE_HELPER(bool, TW_TYPE_BOOLCPP);
ATB_GEN_TYPE_HELPER(char, TW_TYPE_CHAR);
ATB_GEN_TYPE_HELPER(int8_t, TW_TYPE_INT8);
ATB_GEN_TYPE_HELPER(uint8_t, TW_TYPE_UINT8);
ATB_GEN_TYPE_HELPER(int16_t, TW_TYPE_INT16);
ATB_GEN_TYPE_HELPER(uint16_t, TW_TYPE_UINT16);
ATB_GEN_TYPE_HELPER(int32_t, TW_TYPE_INT32);
ATB_GEN_TYPE_HELPER(uint32_t, TW_TYPE_UINT32);
ATB_GEN_TYPE_HELPER(float, TW_TYPE_FLOAT);
ATB_GEN_TYPE_HELPER(double, TW_TYPE_DOUBLE);
ATB_GEN_TYPE_HELPER(std::string, TW_TYPE_STDSTRING);

template<size_t N>
struct TypeHelper<char[N]> {
    static const TwType type_flag = TW_TYPE_CSSTRING(N);
};

struct ICallback {
    virtual ~ICallback() {}
};

typedef std::unique_ptr<ICallback> ICallbackPtr;

struct ISetGetCallbacks: public ICallback {
    virtual void set(const void* value) = 0;

    virtual void get(void* value) = 0;
};

template<typename SetCallback, typename GetCallback>
struct SetGetCallbacks: public ISetGetCallbacks {
    SetCallback setCallback;
    GetCallback getCallback;

    SetGetCallbacks(SetCallback set, GetCallback get):
        setCallback(set), getCallback(get) {
    }

    virtual void set(const void* value) {
        typedef decltype(getCallback()) VariableType;
        setCallback(*(VariableType*) value);
    }

    virtual void get(void* value) {
        typedef decltype(getCallback()) VariableType;
        VariableType* ptr = (VariableType*) value;
        *ptr = getCallback();
    }
};

typedef std::unique_ptr<ISetGetCallbacks> SetGetCallbackPtr;

template<typename SetCallback, typename GetCallback>
SetGetCallbackPtr make_callbacks(SetCallback set, GetCallback get) {
    return SetGetCallbackPtr(new SetGetCallbacks<SetCallback, GetCallback>(set, get));
}

void GenericSet(const void* value, void* clientData);
void GenericGet(void* value, void* clientData);

ISetGetCallbacks* addCallbacks(TwBar* bar, const char* name, SetGetCallbackPtr callbacks);

template<typename T>
void addVarRW(TwBar* bar, const char* name, T& ref, const char* def = "") {
    TwAddVarRW(bar, name, TypeHelper<T>::type_flag, &ref, def);
}

template<typename T>
void addVarRO(TwBar* bar, const char* name, const T& ref, const char* def = "") {
    TwAddVarRO(bar, name, TypeHelper<T>::type_flag, &ref, def);
}

template<typename SetCallback, typename GetCallback>
void addVarCB(TwBar* bar, const char* name, SetCallback set, GetCallback get, const char* def = "") {
    typedef decltype(get()) VariableType;
    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks(set, get));
    TwAddVarCB(bar, name, TypeHelper<VariableType>::type_flag, GenericSet, GenericGet, callbacks, def);
}

// Call the callback when the variable change
template<typename T, typename Callback>
void addVarRWCB(TwBar* bar, const char* name, T& ref, Callback cb, const char* def = "") {
    typedef T VariableType;

    auto set = [&ref, cb](const T& value) {
        ref = value;
        cb();
    };

    auto get = [&ref]() -> T {
        return ref;
    };

    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks(set, get));
    TwAddVarCB(bar, name, TypeHelper<VariableType>::type_flag, GenericSet, GenericGet, callbacks, def);
}

template<typename GetCallback>
void addVarROCB(TwBar* bar, const char* name, GetCallback get, const char* def = "") {
    typedef decltype(get()) VariableType;
    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks([](const VariableType&){}, get));
    TwAddVarCB(bar, name, TypeHelper<VariableType>::type_flag, nullptr,
               GenericGet, callbacks, def);
}

template<typename T>
void addConst(TwBar* bar, const char* name, const T& value, const char* def = "") {
    addVarROCB(bar, name, [value]() -> T { return value; }, def);
}

inline void addLabel(TwBar* bar, const char* name, const char* def = "") {
    addConst(bar, name, std::string(), def);
}

template<typename T, typename Callback>
void addVarRWCB(TwBar* bar, const char* name, const std::string& enumString, T& ref,
             Callback cb, const char* def = "") {
    auto set = [&ref, cb](const T& value) {
        ref = value;
        cb();
    };

    auto get = [&ref]() -> T {
        return ref;
    };

    TwType type = TwDefineEnumFromString(nullptr, enumString.c_str());
    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks(set, get));
    TwAddVarCB(bar, name, type, GenericSet, GenericGet, callbacks, def);
}

template<typename SetCallback, typename GetCallback>
void addVarCB(TwBar* bar, const char* name, const std::string& enumString,
             SetCallback set, GetCallback get, const char* def = "") {
    TwType type = TwDefineEnumFromString(nullptr, enumString.c_str());
    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks(set, get));
    TwAddVarCB(bar, name, type, GenericSet, GenericGet, callbacks, def);
}

template<typename SetCallback, typename GetCallback>
void addVarCB(TwBar* bar, const char* name, const TwEnumVal* enumValues, size_t count,
             SetCallback set, GetCallback get, const char* def = "") {
    TwType type = TwDefineEnum(nullptr, enumValues, count);
    ISetGetCallbacks* callbacks = addCallbacks(bar, name, make_callbacks(set, get));
    TwAddVarCB(bar, name, type, GenericSet, GenericGet, callbacks, def);
}

template<typename T>
void addVarRW(TwBar* bar, const char* name, const TwEnumVal* enumValues, size_t count, T& ref, const char* def = "") {
    TwType type = TwDefineEnum(nullptr, enumValues, count);
    TwAddVarRW(bar, name, type, &ref, def);
}

template<typename T>
void addVarRW(TwBar* bar, const char* name, const std::string& enumString, T& ref, const char* def = "") {
    TwType type = TwDefineEnumFromString(nullptr, enumString.c_str());
    TwAddVarRW(bar, name, type, &ref, def);
}

struct ButtonCallback: public ICallback {
    std::function<void()> callback;

    template<typename Functor>
    ButtonCallback(Functor f): callback(f) {
    }
};

void GenericButtonCallback(void* clientData);

ButtonCallback* addCallback(TwBar *bar, const char *name, ButtonCallback callback);

template<typename Callback>
void addButton(TwBar *bar, const char *name, Callback callback, const char *def = nullptr) {
    ButtonCallback* pCallback = addCallback(bar, name, ButtonCallback(callback));
    TwAddButton(bar, name, GenericButtonCallback, pCallback, def);
}

void removeVar(TwBar *bar, const char *name);

void removeAllVars(TwBar *bar);

void deleteBar(TwBar* bar);

inline void addSeparator(TwBar* bar) {
    TwAddSeparator(bar, nullptr, nullptr);
}

}

#endif
