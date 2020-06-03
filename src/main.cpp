#include <napi.h>
#include <string>
#include <vector>


#ifdef WIN32
#include <windows.h>
    std::vector<std::string> list()
    {
        std::vector<std::string> ports;
        HKEY hKey = nullptr;
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        {
            return ports;
        }
        DWORD index = 0;

        // This is a maximum length of value name, see:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872%28v=vs.85%29.aspx
        enum { MaximumValueNameInChars = 16383 };

        std::vector<char> outputValueName(MaximumValueNameInChars, 0);
        std::vector<char> outputBuffer(MAX_PATH + 1, 0);
        DWORD bytesRequired = MAX_PATH;
        for (;;) {
            DWORD requiredValueNameChars = MaximumValueNameInChars;
            const LONG ret = ::RegEnumValue(hKey, index, &outputValueName[0], &requiredValueNameChars,
                nullptr, nullptr, reinterpret_cast<PBYTE>(&outputBuffer[0]), &bytesRequired);

            if (ret == ERROR_MORE_DATA)
            {
                outputBuffer.resize(bytesRequired / sizeof(wchar_t) + 2, 0);
            }
            else if (ret == ERROR_SUCCESS)
            {
                ports.push_back(&outputBuffer[0]);
                ++index;
            }
            else
            {
                break;
            }
        }
        ::RegCloseKey(hKey);
        return ports;
    }
#else
    std::vector<std::string> list()
    {
        return {};
    }
#endif

class ListAsyncWorker : public Napi::AsyncWorker
{

public:
    ListAsyncWorker(Napi::Env &env)
        : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env))
    {
    }

    virtual ~ListAsyncWorker() {};

    // Executed inside the worker-thread.
    // It is not safe to access JS engine data structure
    // here, so everything we need for input and output
    // should go on `this`.
    void Execute()
    {
        ports = list();
    }

    // Executed when the async work is complete
    // this function will be run inside the main event loop
    // so it is safe to use JS engine data again
    void OnOK() {
        Napi::Array arr = Napi::Array::New(Env(), ports.size());
        for (auto i = 0u; i < ports.size(); ++i)
        {
            arr.Set(i, Napi::String::New(Env(), ports[i].c_str()));
        }
        deferred.Resolve(arr);
    }

    void OnError(Napi::Error const &error) {
        deferred.Reject(error.Value());
    }

    Napi::Promise GetPromise()
    { 
        return deferred.Promise();
    }

private:
    Napi::Promise::Deferred deferred;
    std::vector<std::string> ports;
};


Napi::Promise List(const Napi::CallbackInfo &info)
{
    auto asyncWorker = new ListAsyncWorker(info.Env());
    auto promise = asyncWorker->GetPromise();
    asyncWorker->Queue();
    return promise;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "list"),
                Napi::Function::New(env, List));
    return exports;
}

NODE_API_MODULE(addon, Init)
