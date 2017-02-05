#ifndef __DOGEE_BASE_H_ 
#define __DOGEE_BASE_H_ 

#include <stdint.h>
#include <assert.h>
#include <hash_map>
#include <functional>
#include "Dogee.h"

namespace Dogee
{
	//test code

	extern uint64_t data[4096 * 32];

	template <class Dest, class Source>
	inline Dest bit_cast(const Source& source) {
		static_assert(sizeof(Dest) <= sizeof(Source), "Error: sizeof(Dest) > sizeof(Source)");
		Dest dest;
		memcpy(&dest, &source, sizeof(dest));
		return dest;
	}

	template <class T> struct AutoRegisterObject;

	typedef uint32_t ObjectKey;
	typedef uint32_t FieldKey;
	typedef uint64_t LongKey;
	class DObject
	{
	protected:
		static const int _LAST_ = 0;
		static const int CLASS_ID = 0;
	private:
		ObjectKey object_id;
	public:
		//test code
		ObjectKey GetObjectId()
		{
			return object_id;
		}
		void SetObjectId(ObjectKey ok)
		{
			object_id = ok;
		}
		DObject(ObjectKey obj_id)
		{
			object_id = obj_id;
		}


	};
	extern thread_local DObject* lastobject;

	typedef  std::function<DObject*(ObjectKey)> VirtualReferenceCreator;

	inline std::hash_map<unsigned, VirtualReferenceCreator>& GetClassMap()
	{
		static std::hash_map<unsigned, VirtualReferenceCreator> VirtualReferenceCreators;
		return VirtualReferenceCreators;
	}

	inline void RegisterClass(unsigned cls_id, VirtualReferenceCreator  func)
	{
		GetClassMap()[cls_id] = func;
	}

	inline VirtualReferenceCreator& GetClassById(unsigned cls_id)
	{
		return GetClassMap()[cls_id];
	}

	template<class T> T currentClassDummy();

	template<class T, bool isVirtual = false> class Reference;
	template<typename T> class Array;

	template <typename T>
	class DSMInterface
	{
	public:
		static T get_value(ObjectKey obj_id, FieldKey field_id)
		{
			T ret = *(T*)(data + obj_id * 100 + field_id);
			return ret;
		}


		static void set_value(ObjectKey obj_id, FieldKey field_id, T val)
		{
			*(T*)(data + obj_id * 100 + field_id) = val;
		}

	};


	template <typename T, bool isVirtual>
	class DSMInterface < Reference<T, isVirtual> >
	{
	public:
		static Reference<T, isVirtual> get_value(ObjectKey obj_id, FieldKey field_id)
		{
			ObjectKey key = *(ObjectKey*)(data + obj_id * 100 + field_id);
			Reference<T, isVirtual> ret(key);
			return ret;
		}

		static void set_value(ObjectKey obj_id, FieldKey field_id, Reference<T, isVirtual> val)
		{
			*(ObjectKey*)(data + obj_id * 100 + field_id) = val.GetObjectId();
		}

	};

	template <typename T>
	class DSMInterface < Array<T> >
	{
	public:
		static Array<T> get_value(ObjectKey obj_id, FieldKey field_id)
		{
			ObjectKey key = *(ObjectKey*)(data + obj_id * 100 + field_id);
			Array<T> ret(key);
			return ret;
		}

		static void set_value(ObjectKey obj_id, FieldKey field_id, Array<T> val)
		{
			*(ObjectKey*)(data + obj_id * 100 + field_id) = val.GetObjectId();
		}

	};
	inline int GetClassId(ObjectKey obj_id)
	{
		return data[obj_id * 100 + 97];
	}
	inline void SetClassId(ObjectKey obj_id, int cls_id)
	{
		data[obj_id * 100 + 97] = cls_id;
	}
	template<typename T> class ArrayElement
	{
		ObjectKey ok;
		FieldKey fk;
	public:

		//copy 
		ArrayElement<T>& operator=(ArrayElement<T>& x)
		{
			set(x.get());
			return *this;
		}

		T get()
		{
			return DSMInterface<T>::get_value(ok, fk);
		}

		void set(T& x)
		{
			DSMInterface<T>::set_value(ok, fk, x);
		}
		//read
		operator T()
		{
			return get();
		}

		T operator->()
		{
			return get();
		}

		//write
		T operator=(T x)
		{
			set(x);
			return x;
		}


		ArrayElement(ObjectKey o_key, FieldKey f_key) : ok(o_key), fk(f_key)
		{
		}

	};


	//template<typename T> T dummy(T);
	//template<typename T> ArrayElement<T> dummy(Array<T>);
	template<typename T, FieldKey FieldId> class Value
	{
	private:

		//copy functions are forbidden, you should copy the value like "a->val = b->val +0"
		template<typename T2, FieldKey FieldId2>Value<T, FieldId>& operator=(Value<T2, FieldId2>& x);
		Value<T, FieldId>& operator=(Value<T, FieldId>& x);
	public:
		int GetFieldId()
		{
			return FieldId;
		}


		T get()
		{
			assert(lastobject != nullptr);// "You should use a Reference<T> to access the member"
			T ret = DSMInterface<T>::get_value(lastobject->GetObjectId(), FieldId);
#ifdef DOGEE_DBG
			lastobject = nullptr;
#endif
			return ret;
		}
		//read
		operator T()
		{
			return get();
		}

		T operator->()
		{
			return get();
		}


		//write
		Value<T, FieldId>& operator=(T x)
		{
			assert(lastobject != nullptr);// "You should use a Reference<T> to access the member"
			DSMInterface<T>::set_value(lastobject->GetObjectId(), FieldId, x);
#ifdef DOGEE_DBG
			lastobject = nullptr;
#endif
			return *this;
		}
		Value()
		{
		}

	};

	//a dirty bypass for Value<Array<T>, FieldId>, just to add "operator[]" 
	template<typename T, FieldKey FieldId> class Value<Array<T>, FieldId>
	{
	private:

		//copy functions are forbidden, you should copy the value like "a->val = b->val +0"
		template<typename T2, FieldKey FieldId2>Value<Array<T>, FieldId>& operator=(Value<T2, FieldId2>& x);
		Value<Array<T>, FieldId>& operator=(Value<Array<T>, FieldId>& x);
	public:
		int GetFieldId()
		{
			return FieldId;
		}


		Array<T> get()
		{
			assert(lastobject != nullptr);// "You should use a Reference<T> to access the member"
			Array<T> ret = DSMInterface<Array<T>>::get_value(lastobject->GetObjectId(), FieldId);
#ifdef DOGEE_DBG
			lastobject = nullptr;
#endif
			return ret;
		}
		//read
		operator Array<T>()
		{
			return get();
		}
		ArrayElement<T> operator[](int k)
		{
			Array<T> ret = get();
			return ret.ArrayAccess(k);
		}
		//write
		Value<Array<T>, FieldId>& operator=(Array<T> x)
		{
			assert(lastobject != nullptr);// "You should use a Reference<T> to access the member"
			DSMInterface<Array<T>>::set_value(lastobject->GetObjectId(), FieldId, x);
#ifdef DOGEE_DBG
			lastobject = nullptr;
#endif
			return *this;
		}
		Value()
		{
		}

	};



	template<typename T> class Array
	{
	private:
		ObjectKey object_id;
	public:
		//test code
		ObjectKey GetObjectId()
		{
			return object_id;
		}
		Array(ObjectKey obj_id)
		{
			object_id = obj_id;
		}

		ArrayElement<T> ArrayAccess(int k)
		{
			FieldKey key = k;
			return ArrayElement<T>(object_id, key);
		}

		ArrayElement<T> operator[](int k)
		{
			return ArrayAccess(k);
		}
	};



	template<class T> class Reference<T, false>
	{
		static_assert(!std::is_abstract<T>::value, "T should be non-abstract when using non-virtual Reference.");
	private:
		T obj;
	public:
		T* operator->()
		{
			lastobject = (DObject*)&obj;
			return &obj;
		}
		ObjectKey GetObjectId()
		{
			return obj.GetObjectId();
		}

		template <class T2>
		Reference<T, false>& operator=(Reference<T2, false> x)
		{
			static_assert(std::is_base_of<T, T2>::value, "T2 should be subclass of T.");
			obj.SetObjectId(x->GetObjectId());
			return *this;
		}

		template <class T2>
		Reference<T, false>& operator=(Reference<T2, true> x)
		{
			static_assert(std::is_base_of<T, T2>::value, "T2 should be subclass of T.");
			obj.SetObjectId(x->GetObjectId());
			return *this;
		}

		template <class T2, bool isVirtual>
		Reference(Reference<T2, isVirtual> x) :obj(x.GetObjectId())
		{
			static_assert(std::is_base_of<T, T2>::value, "T2 should be subclass of T.");
		}

		Reference(ObjectKey key) :obj(key)
		{
			static_assert(std::is_base_of<DObject, T>::value, "T should be subclass of DObject.");
		}

	};

	template<class T> class Reference < T, true >
	{
	private:
		T* pobj;
		ObjectKey okey;
		void RefObj(ObjectKey okey)
		{
			if (okey == 0)
			{
				pobj = nullptr;
				return;
			}
			std::function<DObject*(ObjectKey)> func = GetClassById(GetClassId(okey));
			pobj = (T*)(func(okey)); //dynamic_cast<T*> (func(okey));
			assert(pobj);
		}
	public:
		T* operator->()
		{
			if (!pobj) //possible memory leak in multithreaded environment
				RefObj(okey);
			lastobject = (DObject*)pobj;
			return pobj;
		}

		ObjectKey GetObjectId()
		{
			return okey;
		}

		//copy or upcast
		template <class T2, bool isVirtual>
		Reference<T, true>& operator=(Reference<T2, isVirtual> x)
		{
			static_assert(std::is_base_of<T, T2>::value, "T2 should be subclass of T.");
			okey = x.GetObjectId();
			return *this;
		}

		template <class T2, bool isVirtual>
		Reference(Reference<T2, isVirtual> x)
		{
			static_assert(std::is_base_of<T, T2>::value, "T2 should be subclass of T.");
			pobj = nullptr;
			okey = x.GetObjectId();
		}


		Reference(ObjectKey key)
		{
			static_assert(std::is_base_of<DObject, T>::value, "T should be subclass of DObject.");
			pobj = nullptr;
			okey = key;
		}

		~Reference()
		{
			delete pobj;
		}
	};
	//test code
	extern ObjectKey objid;
	template<class T> class Alloc
	{
	private:
		//test code

		static ObjectKey AllocObjectId()
		{
			return objid++;
		}
	public:
		static Array<T>  newarray()
		{
			return Array<T>(AllocObjectId());
		}

		static Reference<T>  tnew()
		{
			ObjectKey ok = AllocObjectId();
			SetClassId(ok, T::CLASS_ID);
			T ret(ok);
			return Reference<T>(ok);
		}

		template<typename P1>
		static Reference<T>  tnew(P1 p1)
		{
			ObjectKey ok = AllocObjectId();
			SetClassId(ok, T::CLASS_ID);
			T ret(ok, p1);
			return Reference<T>(ok);
		}

		template<typename P1, typename P2>
		static Reference<T>  tnew(P1 p1, P2 p2)
		{
			ObjectKey ok = AllocObjectId();
			SetClassId(ok, T::CLASS_ID);
			T ret(ok, p1, p2);
			return Reference<T>(ok);
		}
		/*
		template<typename P1, typename P2, typename P3>
		T*  tnew(P1 p1, P2 p2, P3 p3)
		{
		return new (alloc()) T(p1, p2, p3);
		}

		template<typename P1, typename P2, typename P3, typename P4>
		T*  tnew(P1 p1, P2 p2, P3 p3, P4 p4)
		{
		return new (alloc()) T(p1, p2, p3, p4);
		}

		template<typename P1, typename P2, typename P3, typename P4, typename P5>
		T*  tnew(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
		return new (alloc()) T(p1, p2, p3, p4, p5);
		}*/
	};

	template <class T> inline T* ReferenceObject(T* obj)
	{
		lastobject = (DObject*)obj;
		return obj;
	}

	template <class T> struct AutoRegisterObject
	{
		static DObject* createInstance(ObjectKey key)
		{
			return dynamic_cast<DObject*> (new T(key));
		}
	public:
		AutoRegisterObject()
		{
			static_assert(!std::is_abstract<T>::value, "No need to register abstract class.");
			static_assert(std::is_base_of<DObject, T>::value, "T should be subclass of DObject.");
			RegisterClass(T::CLASS_ID, createInstance);
		}
	};

	template<class TSrc, class TDest>
	inline TDest force_cast(TSrc src)
	{
		return TDest(src.GetObjectId());
	}
}
#endif