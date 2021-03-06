// Dogee.cpp : 定义控制台应用程序的入口点。
//
#ifdef _WIN32
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "DogeeHelper.h" 
#include "DogeeBase.h"
#include "DogeeMacro.h"
#include "DogeeStorage.h"
#include "DogeeRemote.h"
#include "DogeeThreading.h"
#include "DogeeAccumulator.h"
#include "DogeeSharedConst.h"
#include "DogeeString.h"
#include "DogeeDThreadPool.h"
#include <chrono>
#include <memory>
#include <thread>
using namespace Dogee;



class clsb : public DObject
{
	DefBegin(DObject);
public:
	DefEnd();
	virtual void aaaa() = 0;
	clsb(ObjectKey obj_id) : DObject(obj_id)
	{}
};

class clsc : public clsb
{
	DefBegin(clsb);
public:
	DefEnd();
	virtual void aaaa()
	{
		std::cout << "Class C" << std::endl;
	}
	clsc(ObjectKey obj_id) : clsb(obj_id)
	{}
};


class clsd : public clsb
{
	DefBegin(clsb);
public:
	DefEnd();
	virtual void aaaa()
	{
		std::cout << "Class D" << std::endl;
	}
	clsd(ObjectKey obj_id) : clsb(obj_id)
	{}
};

class clss : public DObject
{
	DefBegin(DObject);
public:
	//Def(int, i);
	//Def(Array<float>, arr);
	Def(next, Array<Ref<clss>>);
	//Def(Array<Array<int>>, mat);
	//DefRef(clsb, true, prv);
	DefEnd();
	clss(ObjectKey obj_id) : DObject(obj_id)
	{
	}
};


class clsa : public DObject
{
	DefBegin(DObject);
public:
	Def(i, int);
	Def(arr, Array<double>);
	Def(next, Array<Ref<clsa>>);
	Def(next2, Ref<clsa>);
	Def(mat, Array<Array<int>>);
	Def(prv, Ref<clsb, true>);
	//DefRef (clsb, true, prv);
	DefEnd();
	clsa(ObjectKey obj_id) : DObject(obj_id)
	{
	}
	clsa(ObjectKey obj_id, int a) : DObject(obj_id)
	{

		self->arr = Dogee::NewArray<double>(10);
		self->next = Dogee::NewArray<Ref<clsa>>(10);
		self->mat = Dogee::NewArray<Array<int>>(10);
		self->mat[0] = Dogee::NewArray<int>(10);
		self->arr[2] = a;
	}
};
class clsaa : public clsa
{
	DefBegin(clsa);
public:
	Def(kk, int);
	Def(lll, int);
	Def(ppp, Ref<clsa, true>);
	DefEnd();
	clsaa(ObjectKey obj_id) : clsa(obj_id)
	{
	}

};



template<typename T> void aaa(T* dummy)
{
	std::cout << "Normal" << std::endl;
}
template<> void aaa(clsa * dummy)
{
	std::cout << "Int" << std::endl;
}


DefGlobal(g_i, int);
DefConst(thres, int);
//RegVirt(clsc);
//RegVirt(clsd);


DefGlobal(sem, Ref<DSemaphore>);


void threadfun(uint32_t param)
{
	std::cout << "Start" << g_i <<" "<< thres << std::endl;
	sem->Acquire(-1);
	std::cout << "Second" << g_i << std::endl;
	sem->Acquire(-1);
	std::cout << "Create Thread" << g_i << std::endl << param;
}
RegFunc(threadfun);


template <typename T>
void readtest()
{
	auto ptr2 = Dogee::NewArray<T>(100);
	int last = 23, cur;
	for (int i = 0; i < 10000; i++)
	{
		cur = last * 34 - i * 99 + 9;
		ptr2[i] = cur;
		last = cur;
	}
	T buf[10000];
	ptr2->CopyTo(buf, 0, 10000);
	last = 23;
	for (int i = 0; i < 10000; i++)
	{
		cur = last * 34 - i * 99 + 9;
		last = cur;
		if (buf[i] != cur)
		{
			std::cout << "R ERR" << i << std::endl;
			break;
		}
	}
	std::cout << "R OK" << typeid(T).name() << std::endl;
}

template <typename T>
void writetest()
{
	auto ptr2 = Dogee::NewArray<T>(100);
	ptr2[12] = 123;
	T buf[100];
	int last = 23, cur;
	for (int i = 0; i < 100; i++)
	{
		cur = last * 34 - i * 99 + 9;
		buf[i] = cur;
		last = cur;
	}
	last = 23;
	ptr2->CopyFrom(buf, 0, 100);
	for (int i = 0; i < 100; i++)
	{
		cur = last * 34 - i * 99 + 9;
		last = cur;
		if (ptr2[i] != cur)
		{
			std::cout << "W ERR" << i << std::endl;
			break;
		}
	}
	std::cout << "W OK" << typeid(T).name() << std::endl;
}

template <typename T>
void singlewritetest()
{
	auto ptr2 = Dogee::NewArray<T>(100);
	int last = 23, cur;
	for (int i = 0; i < 100; i++)
	{
		cur = last * 34 - i * 99 + 9;
		ptr2[i] = cur;
		last = cur;
	}
	last = 23;
	
	for (int i = 0; i < 100; i++)
	{
		cur = last * 34 - i * 99 + 9;
		last = cur;
		if (ptr2[i] != cur)
		{
			std::cout << "SW ERR" << i << std::endl;
			break;
		}
	}
	std::cout << "SW OK" << typeid(T).name() << std::endl;
}

void fieldtest()
{
	writetest<int>();
	writetest<float>();
	writetest<double>();
	writetest<long long>();

	readtest<int>();
	readtest<float>();
	readtest<double>();
	readtest<long long>();

	singlewritetest<int>();
	singlewritetest<float>();
	singlewritetest<double>();
	singlewritetest<long long>();

	clsaa AAA(0);
	std::cout << AAA.i.GetFieldId() << std::endl
		<< AAA.arr.GetFieldId() << std::endl
		<< AAA.next.GetFieldId() << std::endl
		<< AAA.mat.GetFieldId() << std::endl
		<< AAA.prv.GetFieldId() << std::endl
		<< AAA.kk.GetFieldId() << std::endl
		<< AAA.lll.GetFieldId() << std::endl
		<< AAA.ppp.GetFieldId() << std::endl
		<< "OBJ FID END" << std::endl;

	Array<int> arri(0);
	std::cout << (arri[1].get_address()==1) << std::endl;
	Array<long long> arrl(0);
	std::cout << (arrl[1].get_address()==2) << std::endl;
	Array<Ref<clsa>> arro(0);
	std::cout << (arro[1].get_address()==1) << std::endl;
	Array<Array<clsa>> arra(0);
	std::cout << (arra[1].get_address()==1) << std::endl;

	auto ptr = Dogee::NewObj<clsa>(12);
	//AutoRegisterObject<clsa> aaaaaa;
	ptr->next[23] = ptr;
	ptr->next[23]->i = 123;
	ptr->arr[0] = 133;
	ptr->arr[0] = ptr->arr[0] + 1;
	ptr->mat[0][45] = 123;
	std::cout << (ptr->i == 123) << std::endl
		<< (ptr->arr[0] == 134) << std::endl
		<< (ptr->mat[0][45]==123) << std::endl;
}


void cache_test()
{
	
	g_i = 123445;
	thres = 345;
	sem = NewObj<DSemaphore>(0);
	std::cout << sem->GetObjectId();
	Ref<DThread> thread = NewObj<DThread>(threadfun, 1, 3232);
	int i;
	std::cin >> i;
	sem->Release();

	std::cin >> i;
	g_i = 31024;
	sem->Release();

	std::cin >> i;
}

void func2(uint32_t i){
	std::cout << i << std::endl;
	Ref<DString> str(i);
	std::cout << g_i << std::endl << (str->getstr());
};

void objecttest();
extern void accutest();
extern void mrtest();


////////////////////////////checkpoint
#include "DogeeCheckpoint.h"
int shared_local = -1;
int slave_done_i = -1;
DefGlobal(mycounter, int);
DefGlobal(barrier, Ref<DCheckpointBarrier>);
void thread_for_checkpoint(uint32_t param)
{
	std::cout << "Slave process = " << slave_done_i << std::endl;
	for (int i = slave_done_i + 1; i < 100; i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		mycounter = mycounter + 2;
		std::cout << (int) mycounter << std:: endl;
		slave_done_i = i;
		barrier->Enter();
	}
}

void main_process()
{
	if (barrier)
		barrier = NewObj<DCheckpointBarrier>(2);
	std::cout << "shared_local : " << shared_local << std::endl;
	Ref<DThread> th = NewObj<DThread>(THREAD_PROC(thread_for_checkpoint), 1, 123);
	for (int i = shared_local + 1; i < 100; i++)
	{
		shared_local = i;
		barrier->Enter();
	}
	th->Join();
	CloseCluster();
}
class MasterCheckPoint:public CheckPoint
{
public:
	SerialDef(shared_local, std::reference_wrapper<int>);
	MasterCheckPoint() :shared_local(::shared_local)
	{}
	void DoRestart()
	{
		main_process();
	}
};
SerialDecl(MasterCheckPoint,shared_local, std::reference_wrapper<int>);

class SlaveCheckPoint :public CheckPoint
{
public:
	SerialDef(slave_done_i, std::reference_wrapper<int>);
	SlaveCheckPoint() :slave_done_i(::slave_done_i)
	{}
};
SerialDecl(SlaveCheckPoint, slave_done_i, std::reference_wrapper<int>);

int main3(int argc, char* argv[])
{
	HelperInitClusterCheckPoint<MasterCheckPoint, SlaveCheckPoint>(argc, argv,"Test");
	main_process();
	return 0;
}
////////////////////////////checkpoint



/////////////////////////thread pool test
#include "DogeeThreadPool.h"
int main_threadpool(int argc, char* argv[])
{
	LThreadPool pool(2);
	for (int i = 0; i < 50; i++)
	{
		auto lam = [](int i){
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << i << std::endl;
			return i + 1;
		};
		pool.submit2(lam,i);
		pool.submit2(lam, i);
	}
	int dummy;
	std::cin >>dummy;
	return 0;
}
////////////////////////thread pool test end

/////////////////////////Dthread pool test
#include "DogeeFunctional.h"
int main_functional(int argc, char* argv[])
{
	DogeeEnv::ThreadPoolConfig::thread_pool_count = -1;
	HelperInitCluster(argc, argv);

	Array<float> arr = NewArray<float>(10000);
	Array<int> arr2 = NewArray<int>(10000);
	float fff = 1.5;
	GetDData(arr, 10000).Map(arr2, 10000,[fff](float data){
		return (int)(data + fff);
	}).Map(arr,10000,[fff](int data){
		return (float)(data / fff);
	});

	for (int i = 0; i < 50; i++)
	{
		int j = i * 2;
		std::vector<DThreadPool::DThreadPoolEvent> events;
		for (int k = 0; k < 5; k++)
		{
			auto event = DogeeEnv::ThreadPoolConfig::thread_pool->submit([j](int i){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				std::cout << i << " " << j << std::endl;
				return i + 1;
			}, i);
			events.push_back(event);
		}
		for (auto e : events)
			e.Wait(-1);
		std::cout << i<< std::endl;
	}
	int dummy;
	std::cin >> dummy;
	return 0;
}
////////////////////////Dthread pool test end


int main2(int argc, char* argv[])
{
	if (argc == 3 && std::string(argv[1]) == "-s")
	{
		RcSlave(atoi(argv[2]));
	}
	else
	{

		std::vector<std::string> hosts = { "", "127.0.0.1"};
		std::vector<int> ports = { 8080, 18080};
		std::vector<std::string> mem_hosts = { "127.0.0.1" };
		std::vector<int> mem_ports = { 11211 };

		RcMaster(hosts, ports, mem_hosts, mem_ports, BackendType::SoBackendMemcached, CacheType::SoNoCache);
		
/*		std::cout << "Init OK" << std::endl;
		//accutest();
		//mrtest();
		cache_test();
		std::string str;
		std::cin >> str;
		CloseCluster(); */

		//DogeeEnv::InitStorage(BackendType::SoBackendMemcached, CacheType::SoNoCache, mem_hosts, mem_ports, mem_hosts, mem_ports, 0);
		//DogeeEnv::InitCurrentThread();
		Ref<DString> str = NewObj<DString>("String");
		g_i = 123445;
		auto func = [](uint32_t i){
			std::cout << i << std::endl;
			Ref<DString> str(i);
			std::cout << g_i<<std::endl<< (str->getstr());
		};
		int lambd = 2132123;
		auto func_comp = [lambd](uint32_t i){
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << i << std::endl;
			Ref<DString> str(i);
			std::cout << lambd << std::endl << (str->getstr());
		};
		//auto th = NewObj<DThread>(THREAD_PROC(func2), 1, str.GetObjectId());
		auto th = NewObj<DThread>(func_comp, 1, str.GetObjectId());
		std::cout << 0 << std::endl << str->getstr() << std::endl;
		//readtest<int>();
		//auto acc = NewObj<DFunctionalAccumulator<int, adder>>(Array<int>(0),0,0);
		th->Join();
		std::cout << "Input any to continue";
		std::string dummy;
		std::cin >> dummy;
		//fieldtest();
		
		CloseCluster();
	}
	
	return 0;
}
namespace Dogee{

	extern void __regarg(int argc, char* argv[]);
	extern void RestartCurrentProcess(std::vector<std::string>& excludes_ip , std::vector<int>& excludes_ports);
	extern void RestartCurrentProcess();
}
int main(int argc, char* argv[])
{
	Dogee::__regarg(argc, argv);
	std::unordered_map < std::string, std::string> param;
	for (int i = 1; i + 1 < argc; i += 2)
	{
		param[argv[i]] = argv[i + 1];
	}

	auto restart_pid = param.find("----restart");
	if (restart_pid != param.end())
	{
		ProcessIdentifier pid = (ProcessIdentifier)atoi(restart_pid->second.c_str());
		std::cout << "Child waiting\n";
		UaWaitForProcess(pid);
		std::cout << "Child exit\n";
		exit(0);
	}
	std::string buf;
	std::cin >> buf;
	RestartCurrentProcess();
	std::cin >> buf;
	return 0;
}

void objecttest()
{
	Ref<clss> dd[1] ;
	auto ptr2 = Dogee::NewArray<Ref<clss>>(1);

	ptr2->CopyTo(dd, 0, 1);
	auto ptr = Dogee::NewObj<clsa>(12);
	//AutoRegisterObject<clsa> aaaaaa;
	ptr->next[0] = ptr;
	ptr->next[0]->i = 123;
	ptr->arr[0] = 133;
	ptr->arr[0] = ptr->arr[0] + 1;
	double buf[10];
	ptr->arr->CopyTo(buf, 0, 10);

	Array<int> arr_int[1];
	ptr->mat[0][2] = 123;
	ptr->mat->CopyTo(arr_int, 0, 1);
	aaa((clsb*)0);
	//Ref<clsa,true> ppp(12);
	Ref<clsc, true> p2 = Dogee::NewObj<clsc>();
	ptr->prv = p2;
	ptr->prv->aaaa();
	Ref<clsd, true> p3 = Dogee::NewObj<clsd>();
	ptr->prv = p3;
	ptr->prv->aaaa();
	g_i = 0;
	Array<int> ppp = Dogee::force_cast<Array<int>>(ptr);
	std::cout << g_i << std::endl;
	std::cout << ptr->mat[0][2] << std::endl;
	std::cout << ptr->arr[0];
}