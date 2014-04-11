#pragma once
#include "Type.h"
#include "any.h"
#include "DependencyProperty.h"
#include "RoutedEvent.h"
#include "RoutedEventArgs.h"
#include "Binding.h"

NS_MPF

//具有依赖属性的对象
class DependencyObject
{
public:
	//创建 DependencyObject 的新实例
	DependencyObject(){}
	virtual ~DependencyObject();

	//获取当前值
	template<typename T>
	T& GetValue(DependencyProperty<T>& property) const
	{
		auto& name = property.GetName();
		//检测动画值
		auto it = animationValues.find(name);
		if (it != animationValues.end())
		{
			return it->second;
		}
		//检测本地值
		it = localValues.find(name);
		if (it != localValues.end())
		{
			return it->second;
		}
		//检测样式值
		it = styleValues.find(name);
		if (it != styleValues.end())
		{
			return it->second;
		}
		//检测通用值
		return GetCommonValue(property);
	}

	//设置本地值
	template<typename T>
	void SetValue(const DependencyProperty<T>& property, T&& value)
	{
		auto& name = property.GetName();
		//检测本地值
		auto it = localValues.find(name);
		if (it != localValues.end())
		{
			if ((const T&)(it->second) != value)
			{
				it->second = value;
				OnValueChange(name);
			}
		}
		else
		{
			localValues.emplace(name, std::move(value));
		}
	}

	//设置本地值
	template<typename T>
	void SetValue(const DependencyProperty<T>& property, const T& value)
	{
		auto& name = property.GetName();
		//检测本地值
		auto it = localValues.find(name);
		if (it != localValues.end())
		{
			if ((const T&)(it->second) != value)
			{
				it->second = value;
				OnValueChange(name);
			}
		}
		else
		{
			localValues.emplace(name, value);
		}
	}

	//添加事件侦听器
	template<typename THandler, typename TArgs>
	void AddEventHandler(const RoutedEvent<THandler>& ent, std::function<void(TArgs&)> handler)
	{
		localEvents.emplace(ent.GetName(), [=](RoutedEventArgs& args)
		{
			handler(*(TArgs*)(&args));
		});
	}

	//添加事件侦听器
	template<typename THandler, typename TArgs>
	void AddStyleEventHandler(const RoutedEvent<THandler>& ent, std::function<void(TArgs&)> handler)
	{
		AddCommonEventHandlers(ent.GetName(), [=](RoutedEventArgs& args)
		{
			handler(*(TArgs*)(&args));
		});
	}

protected:
	template<typename T>
	void SetPropertyChangedHandler(const DependencyProperty<T>& property, std::function<void()> handler)
	{
		auto& name = property.GetName();
		auto it(observers.find(name));

		if (it != observers.end())
		{
			it->second = handler;
		}
		else
		{
			observers.emplace(name, handler);
		}
	}

	template<typename T>
	void ClearPropertyChangedHandler(const DependencyProperty<T>& property)
	{
		auto& name = property.GetName();
		auto it(observers.find(name));

		if (it != observers.end())
		{
			observers.erase(it);
		}
	}

	template<typename T>
	T& GetCommonValue(DependencyProperty<T>& property) const
	{
		auto& val = FindParentCommonValue(property.GetName());
		if (val.isEmpty())
		{
			return property.GetValue();
		}
		return val;
	}
	MPF_API void OnValueChange(const String& name) const;
protected:
	virtual any& FindParentCommonValue(const String& name) const;
	virtual void InvokeParentCommonEventHandlers(const IRoutedEvent& ent, RoutedEventArgs& args) const;
	virtual void AddCommonEventHandlers(const String& name, RoutedEventHandler&& handler) const;
	static void DoEvent(DependencyObject& obj, const IRoutedEvent& ent, RoutedEventArgs& args);
private:
	//属性
	std::unordered_map<String, any> localValues;
	std::unordered_map<String, any> animationValues;
	std::unordered_map<String, any> styleValues;
	std::unordered_map<String, std::function<void()>> observers;
	//事件
	std::unordered_multimap<String, RoutedEventHandler> localEvents;
	std::unordered_multimap<String, RoutedEventHandler> styleEvents;
};

#define DECLARE_UI_VALUES \
	static std::unordered_map<String, any> commonAnimationValues; \
	static std::unordered_map<String, any> commonStyleValues;	  \
	static std::unordered_multimap<String, RoutedEventHandler> commonStyleEvents;\
	static std::unordered_map<String, std::unique_ptr<Binding>> commonStyleBindings;

#define DEFINE_UI_VALUES(CLASS) \
	std::unordered_map<String, any> CLASS::commonAnimationValues; \
	std::unordered_map<String, any> CLASS::commonStyleValues;	  \
	std::unordered_multimap<String, RoutedEventHandler> CLASS::commonStyleEvents;\
	std::unordered_map<String, std::unique_ptr<Binding>> CLASS::commonStyleBindings;

#define DECLARE_UI_FUNCS \
	virtual any& FindParentCommonValue(const String& name) const; \
	virtual void InvokeParentCommonEventHandlers(const IRoutedEvent& ent, RoutedEventArgs& args) const; \
	virtual void AddCommonEventHandlers(const String& name, RoutedEventHandler&& handler) const;
#define DECLARE_PUB_UI_FUNCS \
	template<typename T>	\
	static void SetCommonStyleValue(const DependencyProperty<T>& property, const T& value)	\
	{																				\
		auto& name = property.GetName();											\
		auto it = commonStyleValues.find(name);										\
		if (it != commonStyleValues.end())											\
		{																			\
			if ((const T&)(it->second) != value)										\
			{																		\
				it->second = value;													\
			}																		\
		}																			\
		else																		\
		{																			\
			commonStyleValues.emplace(name, value);									\
		}																			\
	}																				\
	template<typename T>	\
	static void SetCommonStyleValue(const DependencyProperty<T>& property, T&& value)	\
	{																				\
		auto& name = property.GetName();											\
		auto it = commonStyleValues.find(name);										\
		if (it != commonStyleValues.end())											\
		{																			\
			if ((const T&)(it->second) != value)										\
			{																		\
				it->second = value;													\
			}																		\
		}																			\
		else																		\
		{																			\
			commonStyleValues.emplace(name, std::forward<T>(value));				\
		}																			\
	}																				\
	template<typename T>	\
	static void SetCommonStyleBinding(const DependencyProperty<T>& property, std::unique_ptr<Binding>&& binding) \
	{																				\
		auto& name = property.GetName();											\
		auto it = commonStyleBindings.find(name);									\
		if (it != commonStyleBindings.end())										\
		{																			\
			if (it->second.get() != binding.get())									\
			{																		\
				it->second = binding;												\
			}																		\
		}																			\
		else																		\
		{																			\
			commonStyleBindings.emplace(name, std::move(binding));					\
		}																			\
	}

#define DEFINE_UI_FUNCS(CLASS, BASE) \
	any& CLASS::FindParentCommonValue(const String& name) const \
	{															\
		auto it = CLASS::commonAnimationValues.find(name); 		\
		if (it != CLASS::commonAnimationValues.end()) 			\
		{														\
			return it->second;									\
		}														\
		it = CLASS::commonStyleValues.find(name);				\
		if (it != CLASS::commonStyleValues.end())				\
		{														\
			return it->second;									\
		}														\
		auto it2 = CLASS::commonStyleBindings.find(name);		\
		if (it2 != CLASS::commonStyleBindings.end())			\
		{														\
			return it2->second->GetValue();						\
		}														\
		return BASE::FindParentCommonValue(name);				\
	}															\
	void CLASS::InvokeParentCommonEventHandlers(const IRoutedEvent& ent, RoutedEventArgs& args) const \
	{																							 \
		auto it = CLASS::commonStyleEvents.find(ent.GetName()); 								 \
		if (it != CLASS::commonStyleEvents.end()) 										  		 \
		{																				  		 \
			RoutedEventHandler& handler = it->second;				      						 \
			handler(args);																  		 \
		}						   														  		 \
		BASE::InvokeParentCommonEventHandlers(ent, args);								  		 \
	}																					  		 \
	void CLASS::AddCommonEventHandlers(const String& name, RoutedEventHandler&& handler) const	 \
	{																			 				 \
		CLASS::commonStyleEvents.emplace(name, handler); 							  			 \
	}

#define DEFINE_COMMON_ROUTEDEVENT(name, TArgs) \
	AddStyleEventHandler<std::function<void(TArgs&)>, TArgs>(name##Event, [&](TArgs& args){On##name(args);});

NS_ED