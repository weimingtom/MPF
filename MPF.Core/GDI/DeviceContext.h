#pragma once
#include "../../include/visual/NativeWindow.h"

NS_MPF
NS_GDI

//�豸������
class DeviceContext : public Object
{
public:
	//���� DeviceContext ����ʵ��
	DeviceContext();
	//����
	virtual ~DeviceContext();

	//��ȡ HDC
	HDC GetNativeHandle() const mnoexcept;

	//������ɫ

	//�Ӵ��ڿͻ��˻�ȡ
	static std::shared_ptr<DeviceContext> FromWindowClient(HWND hWnd);
	//���������豸������
	static std::shared_ptr<DeviceContext> CreateCompatible(HDC hDC);

	//��ȡ����
	DECLARE_GETTYPE(DeviceContext)
protected:
private:
	HDC hDC = nullptr;
	HWND hWnd = nullptr;
	bool isOwner = false;
	DECLARE_TYPE(DeviceContext)
};

NS_ED
NS_ED