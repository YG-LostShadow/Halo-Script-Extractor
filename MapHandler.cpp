#include "MapHandler.h"

MapHandler::MapHandler()
{
  MapFileName = nullptr;
  pHand = nullptr;
  pMapHand = nullptr;
  pMapView = nullptr;
  ParenthesesCount = 0;
}

MapHandler::~MapHandler()
{
  MapFileName = nullptr;
  pHand = nullptr;
  pMapHand = nullptr;
  pMapView = nullptr;
  ParenthesesCount = 0;
}

void MapHandler::AddToString(const char* cFmt, ...)
{
	va_list mvalist;
	char cBuffer[1024] = "";

	va_start(mvalist, cFmt);
	_vsnprintf_s(cBuffer, sizeof(cBuffer), cFmt, mvalist);
	va_end(mvalist);

	s.append(cBuffer);
}

void MapHandler::ThrowError(char* Error, bool IsWinAPI)
{
  if(IsWinAPI)
    AddToString("Error (%d): %s\r\n", GetLastError(), Error);
  else
    AddToString("Error: %s\r\n", Error);
}

const s_map_header* MapHandler::LoadMapToMemory()
{
  if((pHand = CreateFileA(MapFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)) == INVALID_HANDLE_VALUE)
  {
    ThrowError("Could not open map file.", true);
	  return NULL;
  }

  if((pMapHand = CreateFileMapping(pHand, nullptr, PAGE_READONLY, 0, 0, nullptr)) == NULL)
  {
    ThrowError("Could not create file mapping.", true);
	  return NULL;
  }

  if((pMapView = MapViewOfFileEx(pMapHand, FILE_MAP_READ, 0, 0, 0, 0)) == nullptr)
  {
    ThrowError("Could not map a view of the file.", true);
	  return NULL;
  }

  return (const s_map_header*)pMapView;
}

void MapHandler::UnloadMapFromMemory()
{
  if(!UnmapViewOfFile(pMapView))
    ThrowError("Could not unmap file.", true);

  if(!CloseHandle(pMapHand))
    ThrowError("Could not close mapped file handle.", true);

  if(!CloseHandle(pHand))
    ThrowError("Could not close file.", true);
}

bool MapHandler::InitMap()
{
	const DWORD kTagsBaseAddressXbox =
		0x803A6000;
	const DWORD kTagsBaseAddressPc =
		0x40440000;

	const size_t kHsSyntaxDataOffset = offsetof(s_scenario, ScriptNodeData);
	const size_t kHsStringDataOffset = offsetof(s_scenario, ScriptStrings);
	const size_t kHsGlobalsBlockOffset = offsetof(s_scenario, Globals);
	const size_t kHsScriptsBlockOffset = offsetof(s_scenario, Scripts);

	Map = LoadMapToMemory();

  if(Map == nullptr)
  {
    ThrowError("Could not load map to memory.", false);
    return false;
  }

  const byte* map_base_address = reinterpret_cast<const byte*>(Map);

  TagIndex = reinterpret_cast<const s_tag_index_base*>(map_base_address + Map->TagIndexOffset);
  if (TagIndex == nullptr)
  {
    ThrowError("Tag index is invalid.", false);
    return false;
  }

  const byte* tags_base_address = reinterpret_cast<const byte*>(TagIndex);

  DWORD platform_tags_base_address;
  switch (TagIndex->FirstTagAddress >> 30)
  {
  case 1:
	  platform_tags_base_address = kTagsBaseAddressPc;
	  break;

  case 2:
	  platform_tags_base_address = kTagsBaseAddressXbox;
	  break;

  default:
	  ThrowError("Tag memory is made in unknown configuration", false);
	  return false;
  }

  Tag = reinterpret_cast<const s_tag_instance*>(tags_base_address + (TagIndex->FirstTagAddress - platform_tags_base_address));
  if(Tag == nullptr)
  {
    ThrowError("Tag instance is invalid.", false);
    return false;
  }

  Scenario = reinterpret_cast<const s_scenario*>(tags_base_address + (Tag->Address - platform_tags_base_address));
  if(Scenario == nullptr)
  {
    ThrowError("Scenario is invalid.", false);
    return false;
  }

  ScriptNodeData = reinterpret_cast<const _ScriptNodeData*>(tags_base_address + (Scenario->ScriptNodeData.Address - platform_tags_base_address));
  if(ScriptNodeData == nullptr)
  {
    ThrowError("Script node data is invalid.", false);
    return false;
  }

  Script = reinterpret_cast<const s_hs_script*>(tags_base_address + (Scenario->Scripts.Address - platform_tags_base_address));
  if(Script != nullptr)
  {
    ScriptCount = Scenario->Scripts.Count;
  }

  ScriptStrings = reinterpret_cast<const char*>(tags_base_address + (Scenario->ScriptStrings.Address - platform_tags_base_address));
  if (ScriptStrings == nullptr)
  {
    ThrowError("Script string data is invalid.", false);
    return false;
  }

  Global = reinterpret_cast<const s_hs_global_internal*>(tags_base_address + (Scenario->Globals.Address - platform_tags_base_address));
  if (Global != nullptr)
  {
    GlobalCount = Scenario->Globals.Count;
  }

  if(Scenario->Globals.Count && !Scenario->Scripts.Count)
  {
    ThrowError("Nothing to extract.", false);
    return false;
  }

  return true;
}

void MapHandler::ExtractNodeData(DWORD NodeIndex, DWORD ScriptIndex)
{
  const s_hs_syntax_node* Node = &ScriptNodeData->Node[NodeIndex];


  if(!Node)
    return;

  if(Node->Flags == (1<<s_hs_syntax_node::_dont_gc_bit))
  {
    AddToString("\r\n(");

    if(Node->ChildNode.Salt != 0)
      ExtractNodeData(Node->ChildNode.Index, ScriptIndex);

    AddToString(")");
  }
  else if (Node->IsPrimitiveValue())
  {
    switch(Node->Type)
    {
    case rtype_bool:
		if (Node->Boolean == false)
        AddToString(" false");
		else if (Node->Boolean == true)
        AddToString(" true");
      break;
    case rtype_real:
      AddToString(" %.2f", Node->Real);
      break;
    case rtype_short:
		AddToString(" %d", static_cast<long>(Node->Int16));
      break;
    case rtype_long:
		AddToString(" %d", Node->Int32);
      break;
    case rtype_string:
    case rtype_function_name:
    case rtype_script:
    case rtype_trigger_volume:
    case rtype_cutscene_flag:
    case rtype_cutscene_camera_point:
    case rtype_cutscene_title:
    case rtype_cutscene_recording:
    case rtype_device_group:
    case rtype_ai:
    case rtype_ai_command_list:
    case rtype_starting_profile:
    case rtype_conversation:
    case rtype_navpoint:
    case rtype_hud_message:
    case rtype_object_list:
    case rtype_sound:
    case rtype_effect:
    case rtype_damage:
    case rtype_looping_sound:
    case rtype_animation_graph:
    case rtype_actor_variant:
    case rtype_damage_effect:
    case rtype_object_definition:
    case rtype_game_difficulty:
    case rtype_team:
    case rtype_ai_default_state:
    case rtype_actor_type:
    case rtype_hud_corner:
    case rtype_object:
    case rtype_unit:
    case rtype_vehicle:
    case rtype_weapon:
    case rtype_device:
    case rtype_scenery:
    case rtype_object_name:
    case rtype_unit_name:
    case rtype_vehicle_name:
    case rtype_weapon_name:
    case rtype_device_name:
    case rtype_scenery_name:
      if(s.at(s.length()-1) != '(')
        AddToString(" ");

	  AddToString("%s", ScriptStrings + Node->SourceOffset);
      break;
    default:
      break;
    }
  }
  else if(Node->IsGlobalReference())
    AddToString(" %s", ScriptStrings + Node->SourceOffset);
  else if(Node->IsScriptCall()) // original code tested flags == 1101b
    AddToString(" %s", Script[ScriptIndex].Name); // shouldn't we index with Node->ScriptIndex here?

  if(Node->NextExpression.Id != -1 && Node->NextExpression.Salt != 0)
  {
    ExtractNodeData(Node->NextExpression.Index, ScriptIndex);
  }
  else if(Node->NextExpression.Id == -1 && Node->ChildNode.Id == 0)
  {
  }
}

const char* MapHandler::GetScriptTypeString(WORD ScriptType)
{
  switch(ScriptType)
  {
  case stype_startup:
    return " startup";
  case stype_dormant:
    return " dormant";
  case stype_continuous:
    return " continuous";
  case stype_static:
    return " static";
  case stype_stub:
    return " stub";
  default:
    return NULL;
  }
}

const char* MapHandler::GetScriptReturnTypeString(WORD ReturnType)
{
  switch(ReturnType)
  {
    case rtype_unparsed:
      return " unparsed ";
      break;
    case rtype_special_form:
      return " special form ";
      break;
    case rtype_function_name:
      return " function name ";
      break;
    case rtype_passthrough:
      return " passthrough ";
      break;
    case rtype_void:
      return " void ";
      break;
    case rtype_bool:
      return " bool ";
      break;
    case rtype_real:
      return " real ";
      break;
    case rtype_short:
      return " short ";
      break;
    case rtype_long:
      return " long ";
      break;
    case rtype_string:
      return " string ";
      break;
    case rtype_script:
      return " script ";
      break;
    case rtype_trigger_volume:
      return " trigger volume ";
      break;
    case rtype_cutscene_flag:
      return " cutscene flag ";
      break;
    case rtype_cutscene_camera_point:
      return " cutscene camera point ";
      break;
    case rtype_cutscene_title:
      return " cutscene title ";
      break;
    case rtype_cutscene_recording:
		    return " cutscene_recording ";
      break;
    case rtype_device_group:
		  return " device_group ";
      break;
    case rtype_ai:
		  return " ai ";
      break;
    case rtype_ai_command_list:
		    return " ai_command_list ";
      break;
    case rtype_starting_profile:
		    return " starting_profile ";
      break;
    case rtype_conversation:
		  return " conversation ";
      break;
    case rtype_navpoint:
		    return " navpoint ";
      break;
    case rtype_hud_message:
		    return " hud_message ";
      break;
    case rtype_object_list:
		  return " object_list ";
      break;
    case rtype_sound:
		  return " sound ";
      break;
    case rtype_effect:
		  return " effect ";
      break;
    case rtype_damage:
		  return " damage ";
      break;
    case rtype_looping_sound:
		  return " looping_sound ";
      break;
    case rtype_animation_graph:
		  return " animation_graph ";
      break;
    case rtype_actor_variant:
		  return " actor_variant ";
      break;
    case rtype_damage_effect:
		  return " damage_effect ";
      break;
    case rtype_object_definition:
		  return " object_definition ";
      break;
    case rtype_game_difficulty:
		  return " game_difficulty ";
      break;
    case rtype_team:
		  return " team ";
      break;
    case rtype_ai_default_state:
		  return " ai_default_state ";
      break;
    case rtype_actor_type:
		  return " actor_type ";
      break;
    case rtype_hud_corner:
		  return " hud_corner ";
      break;
    case rtype_object:
		  return " object ";
      break;
    case rtype_unit:
		  return " unit ";
      break;
    case rtype_vehicle:
		  return " vehicle ";
      break;
    case rtype_weapon:
		  return " weapon ";
      break;
    case rtype_device:
		  return " device ";
      break;
    case rtype_scenery:
		  return " scenery ";
      break;
    case rtype_object_name:
		  return " object_name ";
      break;
    case rtype_unit_name:
		  return " unit_name ";
      break;
    case rtype_vehicle_name:
		  return " vehicle_name ";
      break;
    case rtype_weapon_name:
		  return " weapon_name ";
      break;
    case rtype_device_name:
		  return " device_name ";
      break;
    case rtype_scenery_name:
		  return " scenery_name ";
      break;
    default:
      return NULL;
      break;
  }
}

void MapHandler::ClearString()
{
  s.clear();
}

void MapHandler::InitNewGlobalString(DWORD GlobalIndex)
{
  ClearString();

  if(GlobalIndex > 0)
    AddToString("\r\n\r\n");

  AddToString("(global");
  ParenthesesCount = 0;
}

void MapHandler::InitNewScriptString(DWORD ScriptIndex)
{
  ClearString();

  if(ScriptIndex > 0 || GlobalCount > 0)
    AddToString("\r\n\r\n");

  AddToString("(script");
  ParenthesesCount = 0;
}

void MapHandler::AddEndParentheses()
{
  DWORD Count = ParenthesesCount;

  for(DWORD i = 0; i < Count; i++)
  {
    AddToString(")");
    ParenthesesCount--;
  }
}

void MapHandler::ExtractScript()
{
  for(DWORD j = 0; j < GlobalCount; j++)
  {
    InitNewGlobalString(j);

    const char* Type = GetScriptReturnTypeString(Global[j].Type);

    if(Type)
      AddToString(Type);

    AddToString(Global[j].Name);
    ExtractNodeData(Global[j].InitExpressionIndex.Index, j);
    AddToString(")");
    FinalString.append(s);
  }

  for(DWORD i = 0; i < ScriptCount; i++)
  {
    InitNewScriptString(i);

    const char* ScriptType = GetScriptTypeString(Script[i].ScriptType);
    const char* ReturnType = GetScriptReturnTypeString(Script[i].ReturnType);

    if(ScriptType)
      AddToString(ScriptType);

    if(ReturnType)
      AddToString(ReturnType);

    AddToString(Script[i].Name);
    ExtractNodeData(Script[i].RootExpressionIndex.Index, i);
    AddToString(")");
    FinalString.append(s);
  }
}