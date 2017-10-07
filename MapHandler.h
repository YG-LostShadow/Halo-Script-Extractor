#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include "Main.h"

class MapHandler
{
public:
  MapHandler();
  ~MapHandler();

  void AddToString(const char* cFmt, ...);
  void ThrowError(char* Error, bool IsWinAPI);

  void* pHand;
  void* pMapHand;
  void* pMapView;
  const s_map_header* Map;
  const s_tag_instance* Tag;
  bool InitMap();
  const s_map_header* LoadMapToMemory();
  void UnloadMapFromMemory();

  void ExtractScript();
  void ExtractNodeData(DWORD NodeIndex, DWORD ScriptIndex);
  const char* GetScriptTypeString(WORD ScriptType);
  const char* GetScriptReturnTypeString(WORD ReturnType);
  void AddEndParentheses();
  void InitNewGlobalString(DWORD GlobalIndex);
  void InitNewScriptString(DWORD ScriptIndex);
  void ClearString();

  std::string s;
  std::string MapFile;
  std::string FinalString;
  const char* MapFileName;
  DWORD ScriptCount;
  DWORD GlobalCount;
  DWORD ParenthesesCount;

  const s_tag_index_base* TagIndex;
  const s_scenario* Scenario;
  const _ScriptNodeData* ScriptNodeData;
  const s_hs_script* Script;
  const s_hs_global_internal* Global;
  const char* ScriptStrings;
};

#endif /* MAPHANDLER_H */