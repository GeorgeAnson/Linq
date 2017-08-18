#ifndef LINQCPP_H_H_H
#define LINQCPP_H_H_H
#include<boost\algorithm\minmax_element.hpp>
#include<boost\iterator\zip_iterator.hpp>
#include<boost\range.hpp>
#include<boost\range\join.hpp>
#include<boost\range\adaptors.hpp>
#include<boost\range\algorithm.hpp>
using namespace boost;
using namespace boost::adaptors;
#include<numeric>
#include<vector>
#include<map>

/*
 * Auth Anson<br>
 *
 * Linq库开发，包含大部分linq的操作命令，支持基础类型以及自定义类型<br>
 * <strong>开发环境配置：boost</strong>
 * <p>
 *	 1.对象支持性访问方式的进阶修改<br>
 *   2.系列简化辅助函数存在一定问题，是的访问方式并未变得简单<br>
 * <p>
 */
namespace linq
{
	//定义function_traits用于将lambda转换成function
	template<typename Function>
	struct function_traits :public function_traits<decltype(&Function::operator())>{};
	
	
	template<typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...)const>
	{
		typedef std::function<ReturnType(Args...)> function;
	};

	template<typename Function>
	typename function_traits<Function>::function to_function(Function& lambda)
	{
		return static_cast<typename function_traits<Function>::function> (lambda);
	}

	template<typename R>
	class LinqCpp
	{
	public:
		LinqCpp(R& range) :m_linqrange(range){}
		typedef typename R::value_type value_type;
		//过滤操作
		template<typename F>
		auto  where (const F& f) -> LinqCpp<filtered_range<F, R> >
		{
			return LinqCpp<filtered_range<F, R> > (filter(m_linqrange, f));
		}
		//转换操作
		template<typename F>
		auto select (const F& f) -> LinqCpp<transformed_range<typename function_traits<F>::function, R> >
		{
			auto func = to_function(f);
			return LinqCpp<transformed_range<typename function_traits<F>::function, R> > (boost::adaptors::transform(m_linqrange, func));
		}

		auto begin() const -> decltype(std::begin(boost::declval<const R> ()))
		{
			return std::begin(m_linqrange);
		}

		auto end() const -> decltype(std::end(boost::declval<const R> ()))
		{
			return std::end(m_linqrange);
		}

		template<typename F>
		auto first(const F& f) -> decltype(std::find_if(begin(), end(), f))
		{
			return std::find_if(begin(), end(), f);
		}

		template<typename F>
		auto last(const F& f) -> decltype(reverse().first(f))
		{
			return reverse().first(f);
		}

		bool empty()const
		{
			return begin() == end();
		}

		template<typename F>
		auto any(const F& f)const->bool
		{
			return std::any_of(begin(), end(), f);
		}

		template<typename F>
		auto all(const F& f) const -> bool
		{
			return std::all_of(beign(), end(), f);
		}

		//遍历操作
		template<typename F>
		void for_each(const F& f) const
		{
			std::for_each(begin(), end(), f);
		}

		//根据判断式判断是否包含
		template<typename F>
		auto contains(const F& f) const-> bool
		{
			return std::find_if(begin(), end(), f);
		}

		//根据function去重
		template<typename F>
		auto distinct(const F& f) const -> LinqCpp<decltype(unique(m_linqrange, f))>
		{
			return LinqCpp(unique(m_linqrange, f));
		}

		//简单去重
		auto distinct() -> LinqCpp<boost::range_detail::uniqued_range<R> >
		{
			return LinqCpp<uniqued_range<R> > (m_linqrange | uniqued);
		}

		//累加器
		template<typename F>
		auto aggregate(const F& f) const -> value_type
		{
			auto it = begin();
			auto value = *it++;
			return std::accumulate(it, end(), std::move(value), f);
		}

		//算数运算
		auto sum() const -> value_type
		{
			return aggregate(std::plus<value_type> ());
		}

		auto count() const -> value_type
		{
			return std::distance(begin(), end());
		}

		template<typename F>
		auto count(const F& f) const -> value_type
		{
			return std::count_if(begin(), end(), f);
		}

		auto average() -> value_type
		{
			return sum() / count();
		}

		template<typename F>
		auto min(const F& f) const -> value_type
		{
			return *std::min_element(begin(), end(), f); 
		}

		auto min() const -> value_type
		{
			return *std::min_element(begin(), end());
		}

		template<typename F>
		auto max(const F& f) const -> value_type
		{
			return *std::max_element(begin(), end(), f);
		}

		auto max() const -> value_type
		{
			return *std::max_element(begin(), end());
		}

		template<typename F>
		auto minmax(const F& f) const -> decltype(boost::minmax_element(begin(), end(), f))
		{
			return boost::minmax_element(begin(), end(), f);
		}

		auto minmax() const -> decltype(boost::minmax_element(begin(), end()))
		{
			return boost::minmax_element(begin(), end());
		}

		//获取指定索引位置的元素
		template<typename T>
		auto element(T index) const -> decltype(std::next(begin(), index))
		{
			return boost::next(begin(), index);
		}

		//将map中的键放到一个range中
		auto keys() const -> LinqCpp<boost::select_first_range<R> >
		{
			return LinqCpp<boost::select_first_range<R> > (boost::adaptors::keys(m_linqrange));
		}

		//将map中的值放到一个range中
		auto values() const -> LinqCpp<boost::select_second_const_range<R> >
		{
			return LinqCpp<boost::select_second_const_range<R> > (boost::adaptors::values(m_linqrange));
		}

		//反转操作
		auto reverse() -> LinqCpp<boost::reversed_range<R> >
		{
			return LinqCpp<boost::reversed_range<R> > (boost::adaptors::reverse(m_linqrange));
		}

		//获取前面n个元素
		template<typename T>
		auto take(T n) const -> LinqCpp<decltype(slice(m_linqrange, 0, n))>
		{
			return LinqCpp(slice(m_linqrange, 0, n));
		}

		//获取指定范围内的元素
		template<typename T>
		auto take(T start, T end) const -> LinqCpp<decltype(slice(m_linqrange, start, end))>
		{
			return LinqCpp(slice(m_linqrange, start, end));
		}

		//将range转换为vector
		std::vector<value_type> to_vector()
		{
			return std::vector<value_type> (begin(), end());
		}

		//当条件不满足是返回前面所有元素
		template<typename F>
		auto takewhile(const F f) const -> LinqCpp<decltype(boost::make_iterator_range(begin(), std::find_if(begin(), end(), f)))>
		{
			return LinqCpp(boost::make_iterator_range(begin, std::find_if(begin(), end(), f)));
		}

		//获取第n个元素之后的所有元素
		template<typename T>
		auto skip(T n) const -> LinqCpp<decltype(boost::make_iterator_range(begin() + n, end()))>
		{
			return LinqCpp(boost::make_iterator_range(begin() + n, end()));
		}

		template<typename F>
		auto skipwhile(const F& f) const -> LinqCpp<iterator_range<decltype(begin())> >
		{
			return LinqCpp(boost::make_iterator_range(std::find_if_not(begin(), end(), f), end()));
		}

		//按步长挑选元素组成新集合
		template<typename T>
		auto step(T n) -> decltype(stride(m_linqrange, n))
		{
			return stride(m_linqrange, n);
		}

		//直接将指针或者智能指针指向的内容组成新集合
		auto indirect() -> LinqCpp<boost::indirected_range<R> >
		{
			return LinqCpp<boost::indirected_range<R> > (boost::adaptors::indirect(m_linqrange));
		}

		//连接操作
		template<typename R2>
		auto concat(const R2& other) -> LinqCpp<joined_range<R, const R2> >
		{
			return LinqCpp<joined_range<R, const R2> > (boost::join(m_linqrange, other));
		}

		//排除操作
		template<typename R2>
		void except(const R2& other, std::vector<value_type>& resultVecotr)
		{
			std::set_difference(begin(), end(), std::begin(other), std:; end(other), back_inserter(resultVector));
		}

		//包含操作
		template<typename R2>
		bool includes(const R2& other) const
		{
			return std::includes(begin(), end(), std::begin(other), std::end(other));
		}

		//分组操作
		template<typename Fn>
		std::multimap<typename std::result_of<Fn(value_type)>::type, value_type> groupby(const Fn& f)
		{
			typedef decltype(std::declval<Fn> ()(std::declval<value_type> ())) keytype;
			std::multimap<keytype, value_type> mmap;
			std::for_each(begin(), end(), [&mmap, &f](value_type item)
			{
				mmap.insert(std::make_pair(f(item), item));
			});
			return mmap;
		}

		//允许指定键和值函数的分组操作
		template<typename KeyFn, typename ValueFn>
		std::multimap<typename std::result_of<KeyFn(value_type)>::type, typename std::result_of<ValueFn(value_type)>::type> groupby(const KeyFn& fnk, const ValueFn& fnv)
		{
			typedef typename std::result_of<KeyFn(value_type)>::type keytype;
			typedef typename std::result_of<ValueFn(value_type)>::type valtype;

			std::multimap<keytype, valtype> mmap;
			std::for_each(begin(), end(), [&mmap, &fnk, &fnv](value_type item)
			{
				keytype key = fnk(item);
				valtype val = fnv(item);
				mmp.insert(std::make_pair(key, val));
			});
			return mmp;
		}


		//转换操作
		template<typename T>
		auto cast() -> LinqCpp<boost::transformed_range<std::function<T(value_type)>, R> >
		{
			std::function<T(value_type)> f = [](value_type item)
			{
				return static_cast<T> (item);
			};

			return LinqCpp<transformed_range<std::function<T(value_type)>, R> > (select(f));
		}




		//判断操作
		template<typename R2>
		bool equals(const LinqCpp<R2>& other) const
		{
			return count() == other.count() && std::equal(begin(), end(), other.begin());
		}

		template<typename R2, typename F>
		bool equals(const LinqCpp<R2>& other, const F& f) const
		{
			return count() == other.count() && std::equal(begin(), end(), other.begin(), f);
		}

		template<typename R2>
		bool operator==(const LinqCpp<R2>& other) const
		{
			return equals(other);
		}

		template<typename R2>
		bool operator!=(const LinqCpp<R2>& other) const
		{
			return !(*this == other);
		}

		private:
			R m_linqrange;
	};


	//简化range声明
	template<template<typename T> class IteratorRange, typename R>
	using Range = IteratorRange<decltype(std::begin(std::declval<R> ()))>;

	template<typename R>
	using iterator_range = Range<boost::iterator_range, R>;

	//简化定义LinqCpp的辅助函数
	template<typename R>
	LinqCpp<iterator_range<R> > from(const R& range)
	{
		return LinqCpp<iterator_range<R> > (iterator_range<R>(range));
	}

	//合并range
	template<typename... T>
	auto zip(const T&... containers) -> boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))> >
	{
		auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
		auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
		return boost::make_iterator_range(zip_begin, zip_end);
	}
}

#endif