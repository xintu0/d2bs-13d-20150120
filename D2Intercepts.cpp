#include "D2Handlers.h"
#include "D2Ptrs.h"
#include "D2BS.h"
#include "Helpers.h"

void __declspec(naked) RealmPacketRecv_Interception()
{
	__asm
	{
		LEA ECX,DWORD PTR SS:[ESP+4]
		PUSHAD
		CALL RealmPacketRecv
		CMP EAX, 0
		POPAD
		JE Block
		CALL EAX
Block:
		RETN
	}
}

void __declspec(naked) GamePacketReceived_Intercept()
{
	__asm
	{
		pop ebp;
		pushad;

		call GamePacketReceived;
		test eax, eax;

		popad;
		jnz OldCode;

		mov edx, 0;

OldCode:
		call D2NET_ReceivePacket_I;

		push ebp;
		ret;
	}
}

void __declspec(naked) GamePacketSent_Interception() 
{
	__asm
	{
		pushad;
		mov ecx, [esp + 0x2C];
		mov edx, [esp + 0x24];
		call GamePacketSent;
		test eax, eax;
		popad;
		jnz send;
		mov [esp + 0x4], 0;
send:
		jmp D2NET_SendPacket;
	}
}

void __declspec(naked) GameDraw_Intercept()
{
	__asm
	{
		call GameDraw;

		POP ESI
		POP EBX
		POP ECX
		RETN 4
	}
}

void __declspec(naked) GameInput_Intercept()
{
	__asm {
		pushad
		mov ecx, ebx
		call GameInput
		cmp eax, -1
		popad
		je BlockIt
		call D2CLIENT_InputCall_I
		ret

BlockIt:

		xor eax,eax
		ret
	}
}

UnitAny* GetSelectedUnit_Intercept(void)
{
	if(Vars.bClickAction)
	{
		if(Vars.dwSelectedUnitId)
		{
			UnitAny* pUnit = D2CLIENT_FindUnit(Vars.dwSelectedUnitId, Vars.dwSelectedUnitType);

			return pUnit;
		}

		return NULL;
	}

	return D2CLIENT_GetSelectedUnit();
}

void __declspec(naked) Whisper_Intercept()
{
	__asm
	{
		MOV EBP,DWORD PTR SS:[ESP+0x1FC+4]
		pushad
		mov ecx, edx
		mov edx, ebx
		call WhisperHandler
		popad
		//jmp D2MULTI_WhisperIntercept_Jump
		retn
	}
}
VOID __declspec(naked) ChatPacketRecv_Interception()
{
        __asm
        {
                lea     ecx, [esi+4]
                pushad
                mov     edx, ebp
                mov     ecx, esi
 
                call ChatPacketRecv
                test eax, eax
                popad
 
                je Block
                call eax
Block:
                ret
   }
}
void __declspec(naked) GameAttack_Intercept()
{
	__asm 
	{
		push ecx
		mov ecx, [esp+0xC]
		call GameAttack
		pop ecx

		cmp eax, -1
		je OldCode

		call D2CLIENT_GetSelectedUnit
		
		cmp eax, 0
		je OldCode

		mov [esp+0x0C], 1

OldCode:
		mov eax, [p_D2CLIENT_ScreenSizeY]
		mov eax, [eax]
		retn
	}
}

void __declspec(naked) PlayerAssignment_Intercept()
{
	__asm
	{
		FNOP
		CALL D2CLIENT_AssignPlayer_I
		MOV ECX, EAX
		CALL GamePlayerAssignment
		RETN
	}
}

void __declspec(naked) GameCrashFix_Intercept()
{
	__asm
	{
		CMP ECX, 0
		JE Skip
		MOV DWORD PTR DS:[ECX+0x10],EDX
Skip:
		MOV DWORD PTR DS:[EAX+0xC],0
		RETN
	}
}

void GameDrawOOG_Intercept(void)
{
	GameDrawOOG();
}

void __declspec(naked) GameActChange_Intercept(void)
{
	__asm
	{
		POP EAX
		PUSH EDI
		XOR EDI, EDI
		CMP [Vars.bChangedAct], 0
		MOV [Vars.bChangedAct], 0
		JMP EAX
	}
}

void __declspec(naked) GameActChange2_Intercept(void)
{
	__asm
	{
		MOV [Vars.bChangedAct], 1
		retn 4
	}
}

void __declspec(naked) GameLeave_Intercept(void)
{
	__asm
	{
		call GameLeave
		jmp D2CLIENT_GameLeave_I
	}
}

void __declspec(naked) ChannelInput_Intercept(void)
{
	__asm
	{
		push ecx
		mov ecx, esi

		call ChannelInput

		test eax, eax
		pop ecx

		jz SkipInput
		mov eax, dword ptr[esp+4]
		push eax
		call D2MULTI_ChannelInput_I

SkipInput:
		ret 4
	}
}

void __declspec(naked) AddUnit_Intercept(UnitAny* lpUnit)
{
	__asm
	{
		call [D2CLIENT_GameAddUnit_I]
		pushad
		push esi
		call AddUnit
		popad
		retn
	}
}

void __declspec(naked) RemoveUnit_Intercept(UnitAny* lpUnit)
{
	__asm
	{
		pushad
		push dword ptr ds:[esi+edx*4]
		call RemoveUnit
		popad
		mov eax,dword ptr ds:[ecx+0xE4]
		mov DWORD PTR ds:[esi+edx*4], eax
		retn
	}
}


VOID __declspec(naked) __fastcall ClassicSTUB()
{
	*p_BNCLIENT_ClassicKey = Vars.szClassic;
	__asm{
		
		jmp BNCLIENT_DClass;
	}
	
}

VOID __declspec(naked) __fastcall LodSTUB()
{ 
	*p_BNCLIENT_XPacKey = Vars.szLod;
	__asm{
		
		jmp BNCLIENT_DLod;
	}
	
}

WINUSERAPI
int
WINAPI
MessageBoxA(
    __in_opt HWND hWnd,
    __in_opt LPCSTR lpText,
    __in_opt LPCSTR lpCaption,
    __in UINT uType);

int WINAPI LogMessageBoxA_Intercept(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption,
    UINT uType)
{
	char* dllAddrs;

	Log("Error message box, caption: \"%s\", message:\n%s\n%s", lpCaption,
		lpText, dllAddrs = DllLoadAddrStrs());

	free(dllAddrs);

	return MessageBoxA(hWnd, lpText, lpCaption, uType);
}