#include "stdafx.h"
#include "CTaskManager.h"

DWORD   CTaskManager::m_nTlsWorker;

CTaskManager::CTaskManager():m_nNumberOfWorkers(0), m_bStopping(false)
{

}

CTaskManager::~CTaskManager()
{

}

bool CTaskManager::Start(int nNumberOfWorkers)
{
	m_bStopping = false;

	if (nNumberOfWorkers > MAX_WORKERS)
		nNumberOfWorkers = MAX_WORKERS;

	m_nNumberOfWorkers = nNumberOfWorkers;

	//������� ������ ��������� ������ ������ ��� ���������� ���� �������� CWorker
	m_nTlsWorker = TlsAlloc();
	if (m_nTlsWorker == TLS_OUT_OF_INDEXES)
		return false;

	//��� ���� ������� ������ �����
	if (!m_workers[0].Init(this) || !TlsSetValue(this->m_nTlsWorker, this))
		return false;

	//��������� ���������� ������
	for (unsigned nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
	{
		if (!m_workers[nWorker].Start(this))
			return false;
	}

	return true;
}

void CTaskManager::Stop()
{
	m_bStopping = true;

	for (unsigned short nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
	{
		while (!m_workers[nWorker].IsFinished())
		{

		}
	}

	TlsFree(m_nTlsWorker);
	m_nTlsWorker = TLS_OUT_OF_INDEXES;
}

