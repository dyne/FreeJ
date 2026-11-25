#include <freeframe.h>
#include <stdint.h>

plugMainUnion plugMain(DWORD operation, LPVOID argument, DWORD instance) {
  static PlugInfoStruct info = {1, 0, "test", "FF fixture", FF_EFFECT};
  plugMainUnion result = {0};
  if (operation == FF_GETINFO) result.PISvalue = &info;
  else if (operation == FF_GETPLUGINCAPS) result.ivalue = FF_TRUE;
  else if (operation == FF_INSTANTIATE) result.ivalue = 1;
  else if (operation == FF_PROCESSFRAME && instance == 1)
    ((uint32_t *)argument)[0] = UINT32_C(0xaa113344);
  return result;
}
