#ifndef UNICODE
#define UNICODE
#endif

#include <node_api.h>
#include <napi.h>
#include <dshow.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>

#pragma comment(lib, "strmiids")

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker** ppEnum)
{
  // Create the System Device Enumerator.
  ICreateDevEnum* pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

  if (SUCCEEDED(hr))
  {
    // Create an enumerator for the category.
    hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
    if (hr == S_FALSE)
    {
      hr = VFW_E_NOT_FOUND; // The category is empty. Treat as an error.
    }
    pDevEnum->Release();
  }
  return hr;
}

char** DisplayDeviceInformation(IEnumMoniker* pEnum)
{
  IMoniker* pMoniker = NULL;
  char** device_list = (char**)malloc(1 * sizeof(char*));
  int index = 0;
  while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
  {
    IPropertyBag* pPropBag;
    HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
    if (FAILED(hr))
    {
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
      device_list = (char**)realloc(device_list, (index + 1) * sizeof(char*));
      _bstr_t b(var.bstrVal);
      char* c = b;
      device_list[index] = c;
      VariantClear(&var);
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

  return device_list;
}

Napi::Value enumerateDevice(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

  napi_value device_list;
  napi_status status = napi_create_array(env, &device_list);
  char** device_list_dummy = NULL;

  if (SUCCEEDED(hr))
  {
    IEnumMoniker* pEnum;

    // hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
    // if (SUCCEEDED(hr))
    // {
    //   DisplayDeviceInformation(pEnum);
    //   pEnum->Release();
    // }
    hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr))
    {
      device_list_dummy = DisplayDeviceInformation(pEnum);
      pEnum->Release();
    }
    CoUninitialize();
  }

  int len = (int)(sizeof(device_list_dummy) / sizeof(device_list_dummy[0]));
  for (int i = 0; i <= len; i++)
  {
    napi_set_element(env, device_list, i, Napi::String::New(env, (const char*)device_list_dummy[i]));
  }
  Napi::Function cb = info[0].As<Napi::Function>();
  cb.Call(env.Global(), { device_list });
  free(device_list_dummy);
  return Napi::String::New(env, "enumerate device");
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "enumerateDevices"), Napi::Function::New(env, enumerateDevice));
  return exports;
}
NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
