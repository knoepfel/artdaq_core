 // This file (RonVec.hh) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Sep  3, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: RonVec.hh,v $
 // rev="$Revision: 1.8 $$Date: 2014/09/05 19:21:11 $";
#ifndef RonVec_hh
#define RonVec_hh

#include <cassert>
#include <cstddef>		// ptrdiff_t
#include <string.h>		// memcpy
#include <strings.h>		// bzero
#include <utility>		// std::swap 
#include <memory>		// unique_ptr

//#include "trace.h"		// TRACE
#ifndef TRACE
# define TRACE( lvl, ... )
# define UNDEF_TRACE_AT_END
#endif

#define USE_UNIQUE_PTR 0

#ifndef RONVEC_DO_TEMPLATE
# define RONVEC_DO_TEMPLATE    1
#endif
#ifndef RONVEC_TT
# define RONVEC_TT unsigned long
#endif

#undef NOT_OLD_CXXSTD
#if !defined(__GCCXML__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
# define NOT_OLD_CXXSTD 1
#endif

#if RONVEC_DO_TEMPLATE == 0
# define TT_		 RONVEC_TT
# define RONVEC_TEMPLATE
# define RONVEC          RonVec
# define RONVEC_TN       RonVec
# define RONVEC_VERSION
#else
# define RONVEC_TEMPLATE template <typename TT_>
# define RONVEC          RonVec<TT_>
# define RONVEC_TN       typename RonVec<TT_>
# define RONVEC_VERSION  static short Class_Version() { return 3; } // proper version for templates
#endif

RONVEC_TEMPLATE
struct RonVec
{
    typedef       TT_*        iterator;
    typedef const TT_*  const_iterator;
    typedef       TT_&       reference;
    typedef const TT_& const_reference;
    typedef       TT_       value_type;
    typedef ptrdiff_t  difference_type;
    typedef    size_t        size_type;

    RonVec( size_t sz );
    RonVec( size_t sz, TT_ val );
#  if USE_UNIQUE_PTR == 0
    ~RonVec();
#   define PTR_(xx) xx
#  else
#   define PTR_(xx) xx.get()
#  endif

    RonVec( const RonVec & other ) //= delete; // non construction-copyable
	: size_(other.size_), data_(new TT_[other.size_]), capacity_(other.capacity_)
    {	TRACE( 10, "RonVec copy ctor this=%p data_=%p other.data_=%p size_=%d other.size_=%d"
	      , (void*)this, (void*)PTR_(data_), (void*)PTR_(other.data_), size_, other.size_ );
	memcpy( PTR_(data_), PTR_(other.data_), size_*sizeof(TT_) );
    }
    RONVEC & operator=( const RonVec & other ) //= delete; // non copyable
    {	TRACE( 10, "RonVec copy assign this=%p data_=%p other.data_=%p size_=%d other.size_=%d"
	      , (void*)this, (void*)PTR_(data_), (void*)PTR_(other.data_), size_, other.size_ );
	resize( other.size_ );
	memcpy( PTR_(data_), PTR_(other.data_), size_*sizeof(TT_) );
	return *this;
    }
#  if NOT_OLD_CXXSTD
    RonVec( RonVec && other )	 // construction-movable
	: size_(other.size_), data_(std::move(other.data_)), capacity_(other.capacity_)
    {   TRACE( 10, "RonVec move ctor this=%p data_=%p other.data_=%p"
	      , (void*)this, (void*)PTR_(data_), (void*)PTR_(other.data_) );
#      if USE_UNIQUE_PTR == 0
	other.data_ = nullptr;
#      endif
    }
    RONVEC & operator=( RonVec && other ) // assign movable
    {   TRACE( 10, "RonVec move assign this=%p data_=%p other.data_=%p"
	      , (void*)this, (void*)PTR_(data_), (void*)PTR_(other.data_) );
	size_ = other.size_;
	delete [] data_;
	data_ = std::move(other.data_);
	capacity_ = other.capacity_;
#      if USE_UNIQUE_PTR == 0
	other.data_ = nullptr;
#      endif
	return *this;
    }
#  endif

    TT_& operator[](int idx);
    const TT_& operator[](int idx) const;
    size_t size() const;
    size_t capacity() const;
    iterator       begin();
    const_iterator begin() const;
    iterator       end();
    const_iterator end() const;
    void reserve( size_t size );
    void resize( size_t size );
    void resize( size_t size, TT_ val );
    iterator insert( const_iterator position, size_t nn, const TT_& val );
    iterator insert(  const_iterator position, const_iterator first
		    , const_iterator last);
    iterator erase( const_iterator first, const_iterator last );
    void     swap( RonVec& x );
    void push_back( const value_type& val );

    RONVEC_VERSION

private:
    // Root needs the size_ member first. It must be of type int.
    // Root then needs the [size_] comment after data_.
    // Note: NO SPACE between "//" and "[size_]"
    int size_;
#  if USE_UNIQUE_PTR == 0
    TT_ * data_; //[size_]
#  else
    std::unique_ptr<TT_[]> data_;
#  endif
    size_t capacity_;
};

RONVEC_TEMPLATE
inline RONVEC::RonVec( size_t sz )
    : size_(sz), data_(new TT_[sz]), capacity_(sz)
{   TRACE( 15, "RonVec %p ctor sz=%d data_=%p", (void*)this, size_, (void*)PTR_(data_) );
}
RONVEC_TEMPLATE
inline RONVEC::RonVec( size_t sz, TT_ val )
    : size_(sz), data_(new TT_[sz]), capacity_(sz)
{   TRACE( 15, "RonVec %p ctor sz=%d/v data_=%p", (void*)this, size_, (void*)PTR_(data_) );
    for (iterator ii=begin(); ii!=end(); ++ii) *ii=val;
    //bzero( &data_[0], (sz<4)?(sz*sizeof(TT_)):(4*sizeof(TT_)) );
}

#if USE_UNIQUE_PTR == 0
RONVEC_TEMPLATE
inline RONVEC::~RonVec()
{   TRACE( 15, "RonVec %p dtor start data_=%p size_=%d"
	  , (void*)this, (void*)PTR_(data_), size_ );
    delete [] data_;
    TRACE( 15, "RonVec %p dtor return", (void*)this );
}
#endif

RONVEC_TEMPLATE
inline TT_& RONVEC::operator[](int idx)
{   assert(idx<(int)size_); return data_[idx];
}

RONVEC_TEMPLATE
inline const TT_& RONVEC::operator[](int idx) const
{   assert(idx < (int)size_);
    return data_[idx];
}

RONVEC_TEMPLATE
inline size_t RONVEC::size()     const { return size_; }
RONVEC_TEMPLATE
inline size_t RONVEC::capacity() const { return capacity_; }

RONVEC_TEMPLATE
inline RONVEC_TN::iterator       RONVEC::begin()       { return iterator(PTR_(data_)); }
RONVEC_TEMPLATE
inline RONVEC_TN::const_iterator RONVEC::begin() const { return iterator(PTR_(data_)); }
RONVEC_TEMPLATE
inline RONVEC_TN::iterator       RONVEC::end()       { return iterator(PTR_(data_)+size_); }
RONVEC_TEMPLATE
inline RONVEC_TN::const_iterator RONVEC::end() const { return const_iterator(PTR_(data_)+size_); }

RONVEC_TEMPLATE
inline void RONVEC::reserve( size_t size )
{   if (size > capacity_)  // reallocation if true
    {
#      if USE_UNIQUE_PTR == 0
	TT_ * old=data_;
	data_ = new TT_[size];
	memcpy( data_, old, size_*sizeof(TT_) );
	TRACE( 13, "RONVEC::reserve this=%p old=%p data_=%p"
	      , (void*)this, (void*)old, (void*)data_ );
	delete [] old;
#      else
	std::unique_ptr<TT_[]> old=std::move(data_);
	data_ = std::unique_ptr<TT_[]>(new TT_[size]);
	memcpy( data_.get(), old.get(), size_*sizeof(TT_) );
#      endif
	capacity_ = size;
	// bye to old(unique_ptr)
    }
}

RONVEC_TEMPLATE
inline void RONVEC::resize( size_t size )
{   if      (size < (size_t)size_)      size_ = size; // decrease
    else if (size <= capacity_) size_ = size;
    else // increase/reallocate 
    {
#      if USE_UNIQUE_PTR == 0
	TT_ * old=data_;
	data_ = new TT_[size];
	memcpy( data_, old, size_*sizeof(TT_) );
	TRACE( 13, "RONVEC::resize this=%p old=%p data_=%p"
	      , (void*)this, (void*)old, (void*)data_ );
	delete [] old;
#      else
	std::unique_ptr<TT_[]> old=std::move(data_);
	data_ = std::unique_ptr<TT_[]>(new TT_[size]);
	memcpy( data_.get(), old.get(), size_*sizeof(TT_) );
#      endif
	size_ = capacity_ = size;
	// bye to old(unique_ptr)
    }
}
RONVEC_TEMPLATE
inline void RONVEC::resize( size_type size, TT_ val )
{   size_type old_size=size;
    resize( size );
    if (size > old_size)
	for (iterator ii=begin()+old_size; ii!=end(); ++ii) *ii=val;
}

RONVEC_TEMPLATE
inline RONVEC_TN::iterator RONVEC::insert(  const_iterator position
							, size_t nn
							, const TT_& val)
{   assert(position<=end());  // the current end
    size_t offset=position-begin();
    reserve( size_+nn );      // may reallocate and invalidate "position"

    iterator dst=end()+nn;    // for shifting existing data after
    iterator src=end();       // insertion point
    size_t cnt=end()-(begin()+offset);
    while (cnt--) *--dst = *--src;

    dst=begin()+offset;
    size_+=nn;
    while (nn--) *dst++ = val;
    return begin()+offset;
}
RONVEC_TEMPLATE
inline RONVEC_TN::iterator RONVEC::insert(  const_iterator position
				       , const_iterator first
				       , const_iterator last)
{   assert(position<=end());  // the current end
    size_t nn=(last-first);
    size_t offset=position-begin();
    reserve( size_+nn );      // may reallocate and invalidate "position"

    iterator dst=end()+nn;    // for shifting existing data after
    iterator src=end();       // insertion point
    size_t cnt=end()-(begin()+offset);
    while (cnt--) *--dst = *--src;

    dst=begin()+offset;
    size_+=nn;
    while (nn--) *dst++ = *first++;
    return begin()+offset;
}

RONVEC_TEMPLATE
inline RONVEC_TN::iterator RONVEC::erase( const_iterator first
				      ,const_iterator last )
{   assert(last<=end());  // the current end
    size_t nn=(last-first);
    size_t offset=first-begin();

    iterator dst=begin()+offset;    // for shifting existing data from last
    iterator src=dst+nn;            // to first
    size_t cnt=end()-src;
    while (cnt--) *dst++ = *src++;

    size_-=nn;
    return begin()+offset;
}

RONVEC_TEMPLATE
inline void RONVEC::swap( RonVec& x )
{   TRACE( 12, "RONVEC::swap this=%p enter data_=%p x.data_=%p"
	  , (void*)this, (void*)PTR_(data_), (void*)PTR_(x.data_) );
    std::swap( data_, x.data_ );
    std::swap( size_, x.size_ );
    std::swap( capacity_, x.capacity_ );
    TRACE( 12, "RONVEC::swap return data_=%p x.data_=%p"
	  , (void*)PTR_(data_), (void*)PTR_(x.data_) );
}

RONVEC_TEMPLATE
inline void RONVEC::push_back( const value_type& val )
{   if ((size_t)size_ == capacity_)
    {   size_t new_sz=size_ + size_/10 + 1;
	reserve( new_sz );
    }
    *end() = val;
    ++size_;
}

#ifdef UNDEF_TRACE_AT_END
# undef TRACE
#endif
#endif /* RonVec_hh */
