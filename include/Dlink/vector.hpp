#ifndef DLINK_HEADER_VECTOR_HPP
#define DLINK_HEADER_VECTOR_HPP

#include <vector>

#ifdef DLINK_MULTITHREADING
#	include <initializer_list>
#	include <memory>
#	include <utility>

#	include <mutex>
#endif

namespace dlink
{
#ifdef DLINK_MULTITHREADING
	template<typename Ty_, typename Allocator_ = std::allocator<Ty_>>
	class vector
	{
		template<typename Ty_, typename Allocator_>
		friend void swap(vector<Ty_, Allocator_>& lhs, vector<Ty_, Allocator_>& rhs);

	private:
		using data_type_ = std::vector<Ty_, Allocator_>;
		using my_ = vector<Ty_, Allocator_>;

	public:
		using value_type = Ty_;
		using allocator_type = Allocator_;
		using size_type = typename data_type_::size_type;
		using difference_type = typename data_type_::difference_type;
		using reference = typename data_type_::reference;
		using const_reference = typename data_type_::const_reference;
		using pointer = typename data_type_::pointer;
		using const_pointer = typename data_type_::const_pointer;
		using iterator = typename data_type_::iterator;
		using const_iterator = typename data_type_::const_iterator;
		using reverse_iterator = typename data_type_::reverse_iterator;
		using const_reverse_iterator = typename data_type_::const_reverse_iterator;

	public:
		vector() noexcept(noexcept(Allocator_()))
		{}
		explicit vector(const Allocator_& allocator) noexcept
			: data_(allocator)
		{}
		vector(size_type count, const Ty_& value, const Allocator_& allocator = Allocator_())
			: data_(count, value, allocator)
		{}
		explicit vector(size_type count, const Allocator_& allocator = Allocator_())
			: data_(count, allocator)
		{}
		template<typename Iterator_>
		vector(Iterator_ first, Iterator_ last, const Allocator_& allocator = Allocator_())
			: data_(first, last, allocator)
		{}
		vector(const my_& vector)
			: data_(vector.data_)
		{}
		vector(const my_& vector, const Allocator_& allocator)
			: data_(vector.data_, allocator)
		{}
		vector(my_&& vector) noexcept
			: data_(std::move(vector.data_))
		{}
		vector(my_&& vector, const Allocator_& allocator)
			: data_(std::move(vector), allocator)
		{}
		vector(std::initializer_list<Ty_> init, const Allocator_& allocator = Allocator_())
			: data_(init, allocator)
		{}
		~vector() = default;

	public:
		my_& operator=(const my_& other)
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			data_ = other.data_;

			return *this;
		}
		my_& operator=(my_&& other) noexcept(data_ = std::move(other.data_))
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			data_ = std::move(other.data_);

			return *this;
		}
		my_& operator=(std::initializer_list<Ty_> init)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_ = init;

			return *this;
		}
		bool operator==(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ == other.data_;
		}
		bool operator!=(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ != other.data_;
		}
		bool operator>(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ > other.data_;
		}
		bool operator>=(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ >= other.data_;
		}
		bool operator<(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ < other.data_;
		}
		bool operator<=(const my_& other) const
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			return data_ <= other.data_;
		}
		const_reference operator[](size_type index) const
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_[index];
		}
		reference operator[](size_type index)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_[index];
		}

	public:
		void assign(size_type count, const Ty_& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.assign(count, value);
		}
		template<typename Iterator_>
		void assign(Iterator_ first, Iterator_ last)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.assign(first, last);
		}
		void assign(std::initializer_list<Ty_> init)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.assign(init);
		}
		const_reference at(size_type index) const
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.at(index);
		}
		reference at(size_type index)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.at(index);
		}

	public:
		allocator_type get_allocator() const
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.get_allocator();
		}
		bool empty() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.empty();
		}
		size_type size() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.size();
		}
		size_type max_size() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.max_size();
		}
		void reserve(size_type new_capacity)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.reserve(new_capacity);
		}
		size_type capacity() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.capacity();
		}
		void shrink_to_fit()
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.shrink_to_fit();
		}
		void resize(size_type count)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.resize(count);
		}
		void resize(size_type count, const Ty_& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.resize(count, value);
		}

		void clear() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.clear();
		}
		iterator insert(const_iterator pos, const Ty_& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.insert(pos, value);
		}
		iterator insert(const_iterator pos, Ty_&& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.insert(pos, std::move(value));
		}
		iterator insert(const_iterator pos, size_type count, const Ty_& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.insert(pos, count, value);
		}
		template<typename Iterator_>
		iterator insert(const_iterator pos, Iterator_ first, Iterator_ last)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.insert(pos, first, last);
		}
		iterator insert(const_iterator pos, std::initializer_list<Ty_> init)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.insert(pos, init);
		}

		const_reference front() const
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.front();
		}
		reference front()
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.front();
		}
		const_reference back() const
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.back();
		}
		reference back()
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.back();
		}
		const_pointer data() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.data();
		}
		pointer data() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.data();
		}

		iterator begin() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.begin();
		}
		const_iterator begin() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.begin();
		}
		const_iterator cbegin() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.cbegin();
		}
		iterator end() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.end();
		}
		const_iterator end() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.end();
		}
		const_iterator cend() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.cend();
		}
		reverse_iterator rbegin() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.rbegin();
		}
		const_reverse_iterator rbegin() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.rbegin();
		}
		const_reverse_iterator crbegin() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.crbegin();
		}
		reverse_iterator rend() noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.rend();
		}
		const_reverse_iterator rend() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.rend();
		}
		const_reverse_iterator crend() const noexcept
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.crend();
		}

		template<typename... Args_>
		iterator emplace(const_iterator pos, Args_&&... args)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.emplace(pos, std::forward<Args_>(args)...);
		}
		iterator erase(const_iterator pos)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.erase(pos);
		}
		iterator erase(const_iterator first, const_iterator last)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.erase(first, last);
		}
		void push_back(const Ty_& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.push_back(value);
		}
		void push_back(Ty_&& value)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.push_back(std::move(value));
		}
		template<typename... Args_>
		reference emplace_back(Args_&&... args)
		{
			std::lock_guard<std::mutex> guard(mutex_);

			return data_.emplace_back(std::forward<Args_>(args)...);
		}
		void pop_back()
		{
			std::lock_guard<std::mutex> guard(mutex_);

			data_.pop_back();
		}

		void swap(my_& other) noexcept(data_.swap(other.data_))
		{
			std::lock_guard<std::mutex> guard(mutex_);
			std::lock_guard<std::mutex> guard_other(other.mutex_);

			data_.swap(other.data_);
		}

	protected:
		const data_type_& data_stl() const noexcept
		{
			return data_;
		}
		data_type_& data_stl() noexcept
		{
			return data_;
		}
		std::mutex& mutex() const noexcept
		{
			return mutex_;
		}

	private:
		data_type_ data_;
		mutable std::mutex mutex_;
	};
#else
	template<typename Ty_, typename Allocator_ = std::allocator<Ty_>>
	using vector = std::vector<Ty_, Allocator_>;
#endif

	template<typename Ty_, typename Allocator_>
	void swap(vector<Ty_, Allocator_>& lhs, vector<Ty_, Allocator_>& rhs)
	{
#ifdef DLINK_MULTITHREADING
		std::lock_guard<std::mutex> guard(lhs.mutex_);
		std::lock_guard<std::mutex> guard_other(rhs.mutex_);

		std::swap(lhs.data_, rhs.data_);
#else
		std::swap(lhs, rhs);
#endif
	}
}

#endif