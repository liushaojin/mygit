#define DLLIMPORT __declspec(dllimport)

DLLIMPORT BOOL DisableTaskKeys(BOOL bEnable, BOOL bBeep);
DLLIMPORT BOOL AreTaskKeysDisabled();