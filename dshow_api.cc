#ifndef UNICODE
#define UNICODE
#endif

#include <node_api.h>
#include <napi.h>
#include <dshow.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
#include <string>
#include <codecvt>

#pragma comment(lib, "strmiids")

std::string utf8_encode(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}


HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
  // Create the System Device Enumerator.
  ICreateDevEnum *pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

  if (SUCCEEDED(hr))
  {
    // Create an enumerator for the category.
    hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
    if (hr == S_FALSE)
    {
      printf("ERROR WITH CreateClassEnumerator\n");
      hr = VFW_E_NOT_FOUND; // The category is empty. Treat as an error.
    }
    pDevEnum->Release();
  }
  else
  {
    printf("ERROR WITH CoCreateInstance\n");
  }
  return hr;
}

Napi::Array enumerateDevice(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  napi_status status;
  Napi::Array device_list = Napi::Array::New(env);
  if (FAILED(hr))
  {
    CoUninitialize();
    // Already initialized
  }
  IEnumMoniker *pEnum;
  hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
  if (SUCCEEDED(hr))
  {
    IMoniker *pMoniker = NULL;
    int index = 0;
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
      IPropertyBag *pPropBag;
      HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
      if (FAILED(hr))
      {
        printf("ERROR WITH BINDING STORAGE\n");
        pMoniker->Release();
        continue;
      }

      VARIANT var;
      VariantInit(&var);

      // Get description or friendly name.
      hr = pPropBag->Read(L"Description", &var, 0);
      if (FAILED(hr))
      {
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
      }
      if (SUCCEEDED(hr))
      {
        std::string c = utf8_encode(var.bstrVal);
        // std::wstring_convert
        status = napi_set_element(env, device_list, index, Napi::String::New(env, c));
        index++;
      }

      // hr = pPropBag->Write(L"FriendlyName", &var);
      // // WaveInID applies only to audio capture devices.
      // hr = pPropBag->Read(L"WaveInID", &var, 0);
      // if (SUCCEEDED(hr))
      // {
      //   printf("WaveIn ID: %d\n", var.lVal);
      //   VariantClear(&var);
      // }
      // hr = pPropBag->Read(L"DevicePath", &var, 0);
      // if (SUCCEEDED(hr))
      // {
      //   // The device path is not intended for display.
      //   printf("Device path: %S\n", var.bstrVal);
      //   VariantClear(&var);
      // }

      pPropBag->Release();
      pMoniker->Release();
    }
    pEnum->Release();
  }
  else
  {
    printf("Error with \"EnumerateDevices\"\n");
  }
  CoUninitialize();

  return device_list;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "enumerateDevices"), Napi::Function::New(env, enumerateDevice));
  return exports;
}
NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
