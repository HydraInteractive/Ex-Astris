#pragma once

// From https://github.com/rigtorp/Function/blob/master/Function.h
/*
	The MIT License (MIT)
	Copyright (c) 2015 Erik Rigtorp <erik@rigtorp.se>
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <functional>
#include <memory>

namespace Hydra::Ext {
	template <class, size_t MaxSize = 2048> class Function;

	template <class R, class... Args, size_t MaxSize> class Function<R(Args...), MaxSize> {
	public:
		Function() noexcept {}

		Function(std::nullptr_t) noexcept {}

		Function(const Function &other) {
			if (other) {
				other.manager(data, other.data, Operation::Clone);
				invoker = other.invoker;
				manager = other.manager;
			}
		}

		Function(Function &&other) { other.swap(*this); }

		template <class F, long unsigned int FSize = sizeof(typename std::decay<F>::type)> Function(F &&f) {
			using f_type = typename std::decay<F>::type;
			static_assert(alignof(f_type) <= alignof(Storage), "invalid alignment");
			static_assert(sizeof(f_type) <= sizeof(Storage) /* aka MaxSize - sizeof(invoker) - sizeof(manager) */ , "storage too small");
			new (&data) f_type(std::forward<F>(f));
			invoker = &invoke<f_type>;
			manager = &manage<f_type>;
		}

		~Function() {
			if (manager) {
				manager(&data, nullptr, Operation::Destroy);
			}
		}

		Function &operator=(const Function &other) {
			Function(other).swap(*this);
			return *this;
		}

		Function &operator=(Function &&other) {
			Function(std::move(other)).swap(*this);
			return *this;
		}

		Function &operator=(std::nullptr_t) {
			if (manager) {
				manager(&data, nullptr, Operation::Destroy);
				manager = nullptr;
				invoker = nullptr;
			}
			return *this;
		}

		template <typename F> Function &operator=(F &&f) {
			Function(std::forward<F>(f)).swap(*this);
			return *this;
		}

		template <typename F> Function &operator=(std::reference_wrapper<F> f) {
			Function(f).swap(*this);
			return *this;
		}

		void swap(Function &other) {
			std::swap(data, other.data);
			std::swap(manager, other.manager);
			std::swap(invoker, other.invoker);
		}

		explicit operator bool() const noexcept { return !!manager; }

		R operator()(Args... args) {
			if (!invoker) {
				throw std::bad_function_call();
			}
			return invoker(&data, std::forward<Args>(args)...);
		}

	private:
		enum class Operation { Clone, Destroy };

		using Invoker = R (*)(void *, Args &&...);
		using Manager = void (*)(void *, void *, Operation);
		using Storage = typename std::aligned_storage<MaxSize - sizeof(Invoker) - sizeof(Manager), 8>::type;

		template <typename F>
		static R invoke(void *data, Args &&... args) {
			F &f = *static_cast<F *>(data);
			return f(std::forward<Args>(args)...);
		}

		template <typename F>
		static void manage(void *dest, void *src, Operation op) {
			switch (op) {
			case Operation::Clone:
				new (dest) F(*static_cast<F *>(src));
				break;
			case Operation::Destroy:
				static_cast<F *>(dest)->~F();
				break;
			}
		}

		Storage data;
		Invoker invoker = nullptr;
		Manager manager = nullptr;
	};
};

/*
 * You may wonder why I have a functor interface, and I can answer that question.
 * Because C++ sucks and I *need* delegates (lambda with capture).
 *
 * Why the fuck would a language specify lambdas but still not specify a type
 * for them. I refuse to use std::function because it is tooooo slow!
 * std::function is banned in Hydra and all games that are powered by it.
 *
 * Only function pointers and functors are allowed!
 */

/*
template<class Sig>
struct IFunctor;

template<class R, class... Args>
struct IFunctor<R(Args...)> {
	inline virtual ~IFunctor() {};
	virtual R operator()(Args...args) const = 0;
};

template<class R, class... Args>
struct FPFunctor : public IFunctor<R, Args...> {
	R (*fp)(Args...);
	inline virtual ~FPFunctor() {}
	inline R operator()(Args... args) { return fp(args...); }
};

template<class Sig>
struct Functor;

template<class R, class... Args>
struct Functor<R(Args...)> {
	enum class StorageType {
		pointer, // Free on dtor
		ref, // Do nothing
		fp // Store in storage
	} _storageType;
	union Storage {
		FPFunctor<R, Args...> fpFunctor;

		inline Storage(FPFunctor<R, Args...>& fpFunctor) : fpFunctor(fpFunctor) {}
	} _storage;
	IFunctor<R, Args...>* functor = nullptr;

	inline Functor() = default;
	inline Functor(std::nullptr_t) {}
	inline Functor(Functor const&) = default;
	virtual ~Functor() {
		switch (_storageType) {
		case StorageType::pointer:
			delete functor;
			break;
		case StorageType::ref:
		case StorageType::fp:
			break;
		}
	}
	inline Functor& operator=(Functor const&) = default;

	/// The Functor object will now own that IFunctor pointer
	inline Functor(IFunctor<R, Args...>* functor) : _storageType(StorageType::pointer), functor(functor) {}

	/// The Functor object expects that the IFunctor reference will live until the end of the lifetime of this object
	inline Functor(IFunctor<R, Args...>& functor) : _storageType(StorageType::ref), functor(&functor) {}

	// TODO: Move constructor?
	// Functor(IFunctor<R, Args...>&& functor) : functor(&functor) {}

	/// The Functor object will 'clone' the function pointer.
	inline Functor(R (*fp)(Args...)) : _storageType(StorageType::fp), _storage(FPFunctor<R, Args...>(fp)),	functor(&_storage.fpFunctor) {}

	inline R operator()(Args... args) { return (*functor)(args...); }

	inline operator bool() { return !!functor; }

	inline void valid() { return !!functor; }
};
*/
