// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� PRINCDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// PRINCDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef PRINCDLL_EXPORTS
#define PRINCDLL_API __declspec(dllexport)
#else
#define PRINCDLL_API __declspec(dllimport)
#endif

//functions

extern "C" PRINCDLL_API void ServiceMain(int argc, wchar_t* argv[]);
void __stdcall ServiceHandler(DWORD dwControl);
DWORD WINAPI main(wchar_t *lpServiceName);
int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress );
