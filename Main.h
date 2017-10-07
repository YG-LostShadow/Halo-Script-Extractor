#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>

struct Entity;
struct DataHeader;
struct s_tag_instance;
struct s_tag_reference;
struct s_tag_block;
struct s_scenario;
struct s_hs_script;
struct _ScriptNodeData;
struct s_hs_syntax_node;

enum ScriptType
{
  stype_startup,
  stype_dormant,
  stype_continuous,
  stype_static,
  stype_stub
};

enum ReturnType
{
  rtype_unparsed,
  rtype_special_form,
  rtype_function_name,
  rtype_passthrough,
  rtype_void,
  rtype_bool,
  rtype_real,
  rtype_short,
  rtype_long,
  rtype_string,
  rtype_script,
  rtype_trigger_volume,
  rtype_cutscene_flag,
  rtype_cutscene_camera_point,
  rtype_cutscene_title,
  rtype_cutscene_recording,
  rtype_device_group,
  rtype_ai,
  rtype_ai_command_list,
  rtype_starting_profile,
  rtype_conversation,
  rtype_navpoint,
  rtype_hud_message,
  rtype_object_list,
  rtype_sound,
  rtype_effect,
  rtype_damage,
  rtype_looping_sound,
  rtype_animation_graph,
  rtype_actor_variant,
  rtype_damage_effect,
  rtype_object_definition,
  rtype_game_difficulty,
  rtype_team,
  rtype_ai_default_state,
  rtype_actor_type,
  rtype_hud_corner,
  rtype_object,
  rtype_unit,
  rtype_vehicle,
  rtype_weapon,
  rtype_device,
  rtype_scenery,
  rtype_object_name,
  rtype_unit_name,
  rtype_vehicle_name,
  rtype_weapon_name,
  rtype_device_name,
  rtype_scenery_name
};

struct Entity
{
	union
	{
    int Id;
    float real;

    struct
    {
		  short Index;
		  short Salt;
    };
	};
};

struct DataHeader
{
	char         Name[32];
	short        Max;		    // Maximum number of instances possible
	short        Size;		  // Size of each instance
	bool         IsValid;
	bool         IdentifierZeroInvalid;
	short        Pad;
	DWORD Signature; // d@t@
	short        NextIndex;
	short        LastIndex;
	Entity       Next;			// the next instance to be initialized
	union
	{
		DWORD First;
//		PVOID       pFirst;	  // Pointer to the first instance
	};
};

struct s_map_header
{
	enum
	{
		k_xbox_version = 5,
		k_pc_version = 7,
		k_ce_version = 609,

		k_stubbs_version = k_xbox_version,
	};

	DWORD HeadSignature;			// 'deah' translates as head
	DWORD Version;			// 5 = Xbox, 6 = Trial, 7 = PC, 609 = CE
	DWORD DecompLen;			// Actual len of decompressed data. Halo sticks garbage on the end so that the file is one of several fixed sizes (35, etc).
	DWORD Unknown0;
	DWORD TagIndexOffset;
	DWORD TagBufferSize;
	DWORD Unknown1[2];			// both always 0x0
	char MapName[32];
	char BuildDate[32];		// Year.Month.Day.Build - I guess they use this to make sure that a certain build will only open that build's map files, because this string is in the app too
	WORD MapType;			// 0 = singleplayer, 1 = multiplayer, 2 = ui - this also determines the size of the cache file. UI = 35MB, multiplayer = 47MB, and singleplayer = 270MB
	WORD Unknown2;
	DWORD Checksum;
	DWORD Unknown3[485];
	DWORD FootSignature;			// 'toof' translates to foot
}; static_assert(sizeof(s_map_header) == 0x800, "s_map_header");

struct s_tag_index_base
{
	union
	{
		DWORD		 FirstTagAddress;	// 0x00
		//		TagInstance* FirstTag;			// 0x00
	};
	Entity			ScenarioIndex;			// 0x04
	DWORD			Checksum;			// 0x08
	unsigned long	TagCount;			// 0x0C
	unsigned long	VertexCount;		// 0x10
	unsigned long	VertexOffset;		// 0x14
	unsigned long	IndicesCount;		// 0x18
	unsigned long	IndicesOffest;		// 0x1C
}; static_assert(sizeof(s_tag_index_base) == 0x20, "s_tag_index_base");

struct s_tag_index_halo : s_tag_index_base
{
	unsigned long	Signature;
}; static_assert(sizeof(s_tag_index_halo) == 0x24, "s_tag_index_halo");

struct s_tag_index_halopc : s_tag_index_base
{
	unsigned long	TotalVertexAndIndicesSize;				// 0x20
	unsigned long	Signature;			// 0x24		'tags'
}; static_assert(sizeof(s_tag_index_halopc) == 0x28, "s_tag_index");

struct s_tag_instance
{
	unsigned long	TagGroup;		// 0x00
  unsigned long	TagChild;		// 0x04
	unsigned long	TagParent;		// 0x08
	Entity			Tag;				// 0x0C
	DWORD			Name;				// 0x10
	DWORD		Address;			// 0x14
	DWORD			Location;			// 0x18
	long			_Unused;			// 0x1C
}; static_assert(sizeof(s_tag_instance) == 0x20, "s_tag_instance");

struct s_tag_reference
{
	unsigned long	TagGroup;		// 0x00
	DWORD			    Name;				// 0x04
	long			    NameLength;	// 0x08
	Entity			  Tag;        // 0x0C
}; static_assert(sizeof(s_tag_reference) == 0x10, "s_tag_block");

struct s_tag_block
{
	unsigned long	Count;				// 0x00
	DWORD		      Address;      // 0x04
	DWORD Definition;
}; static_assert(sizeof(s_tag_block) == 0xC, "s_tag_block");

struct s_tag_data
{
	DWORD Size;
	DWORD Unused0;
	DWORD StreamOffset;
	DWORD Address;
	DWORD Definition;
}; static_assert(sizeof(s_tag_data) == 0x14, "s_tag_data");

struct s_hs_script
{
  char     Name[0x20];          // 0x00
  unsigned short ScriptType;          // 0x20
  unsigned short ReturnType;          // 0x22
  Entity         RootExpressionIndex; // 0x24
  unsigned char  Pad[0x34];           // 0x28
};

struct s_hs_syntax_node
{
	enum
	{
		_primitive_bit,
		_script_index_bit,
		_global_index_bit,
		_dont_gc_bit,
	};

	short Salt;
	union
	{
		short ConstantType;
		short FunctionIndex;
		short ScriptIndex;
	};
	short Type;
	union
	{
		unsigned short Flags;
		short PointerType;
	};
	Entity NextExpression;
	DWORD SourceOffset;
	union
	{
		bool Boolean;
		float Real;
		short Int16;
		long Int32;
		Entity ChildNode;
		DWORD Address;
	};

	bool IsPrimitiveValue() const
	{
		return (Flags & (1<<_primitive_bit)) != 0;
	}
	bool IsScriptCall() const
	{
		return (Flags & (1<<_script_index_bit)) != 0;
	}
	bool IsGlobalReference() const
	{
		return (Flags & (1<<_global_index_bit)) != 0;
	}
	bool IsGarbageCollectable() const
	{
		return (Flags & (1<<_dont_gc_bit)) == 0;
	}
}; static_assert(sizeof(s_hs_syntax_node) == 0x14, "s_hs_syntax_node");

struct _ScriptNodeData
{
  DataHeader ScriptNodeHeader;
  s_hs_syntax_node Node[0/*19001*/];
};

struct s_hs_global_internal
{
  char Name[0x20];
  unsigned int Type;
  unsigned int Unknown;
  Entity InitExpressionIndex;
  unsigned char Pad[0x30];
};

struct s_scenario
{
	s_tag_reference DontUse;				// 0x0000 'sbsp'
	s_tag_reference WontUse;				// 0x0010 'sbsp'
	s_tag_reference CantUse;				// 0x0020 'sky '
  s_tag_block Skies;					// 0x0030
  unsigned short Type;						// 0x003C
  unsigned short Flags;						// 0x003E
	s_tag_block ChildScenarios;			// 0x0040
	float LocalNorth;					// 0x004C // Radians
	unsigned char Unknown00[156];				// 0x0050
	s_tag_block PredictedResources;		// 0x00EC
	s_tag_block Functions;				// 0x00F8
	s_tag_data EditorScenarioData;			// 0x0104
	s_tag_block Comments;					// 0x0118
	unsigned char Unknown04[224];				// 0x0124
	s_tag_block ObjectNames;				// 0x0204
	s_tag_block Scenery;					// 0x0210
	s_tag_block SceneryPalette;			// 0x021C
	s_tag_block Bipeds;					// 0x0228
	s_tag_block BipedPalette;			// 0x0234
	s_tag_block Vehicles;				// 0x0240
	s_tag_block VehiclePalette;			// 0x024C
	s_tag_block Equipment;				// 0x0258
	s_tag_block EquipmentPalette;		// 0x0264
	s_tag_block Weapons;					// 0x0270
	s_tag_block WeaponPalette;			// 0x027C
	s_tag_block DeviceGroups;			// 0x0288
	s_tag_block Machines;				// 0x0294
	s_tag_block MachinePalette;			// 0x02A0
	s_tag_block Controls;				// 0x02AC
	s_tag_block ControlPalette;			// 0x02B8
	s_tag_block LightFixtures;			// 0x02C4
	s_tag_block LightFixturesPalette;	// 0x02D0
	s_tag_block SoundScenery;			// 0x02DC
	s_tag_block SoundSceneryPalette;		// 0x02E8
	unsigned char Unknown05[84];				// 0x02F4
	s_tag_block PlayerStartingProfile;	// 0x0348
	s_tag_block PlayerStartingLocations;	// 0x0354
	s_tag_block TriggerVolumes;			// 0x0360
	s_tag_block RecordedAnimations;		// 0x036C
	s_tag_block NetGameFlags;			// 0x0378
	s_tag_block NetGameEquipment;		// 0x0384
	s_tag_block StartingEquipment;		// 0x0390
	s_tag_block BspSwitchTriggerVolumes;	// 0x039C
	s_tag_block Decals;					// 0x03A8
	s_tag_block DecalPalette;			// 0x03B4
	s_tag_block DetailObjectCollectionPalette;	// 0x03C0
	unsigned char Unknown06[84];				// 0x03CC
	s_tag_block ActorPalette;			// 0x0420
	s_tag_block Encounters;				// 0x042C
	s_tag_block CommandLists;			// 0x0438
	s_tag_block AIAnimationReferences;	// 0x0444
	s_tag_block AIScriptReferences;		// 0x0450
	s_tag_block AIRecordingReferences;	// 0x045C
	s_tag_block AIConversations;			// 0x0468
	s_tag_data ScriptNodeData;				// 0x0474
	s_tag_data ScriptStrings;			// 0x0488
	s_tag_block Scripts;					// 0x049C
	s_tag_block Globals;					// 0x04A8
	s_tag_block References;				// 0x04B4
	s_tag_block SourceFiles;				// 0x04C0
	unsigned char Unknown09[24];				// 0x04CC
	s_tag_block CutsceneFlags;			// 0x04E4
	s_tag_block CutsceneCameraPoints;	// 0x04F0
	s_tag_block CutsceneTitles;			// 0x04FC
	unsigned char Unknown10[108];				// 0x0508
	s_tag_reference CustomObjectNames;		// 0x0574
	s_tag_reference InGameHelpText;		// 0x0584
	s_tag_reference HudMessages;			// 0x0594
	s_tag_block StructureBSPs;			// 0x05A4
};	static_assert(sizeof(s_scenario) == 0x5B0, "s_scenario");


#endif /* MAIN_H */