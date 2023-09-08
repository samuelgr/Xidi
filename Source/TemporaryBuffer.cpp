/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file TemporaryBuffer.cpp
 *   Partial implementation of temporary buffer management functionality.
 **************************************************************************************************/

#include "TemporaryBuffer.h"

#include <cstdint>
#include <cstdlib>
#include <mutex>

namespace Xidi
{
  /// Holds all static data associated with the management of temporary buffers.
  /// Used to make sure that temporary buffer functionality is available as early as dynamic
  /// initialization. Implemented as a singleton object.
  class TemporaryBufferData
  {
  public:

    /// Statically-allocated buffer space itself.
    uint8_t staticBuffers[TemporaryBufferBase::kBuffersTotalNumBytes];

    /// List of free statically-allocated buffers.
    uint8_t* freeBuffers[TemporaryBufferBase::kBuffersCount];

    /// Index of next valid free buffer list element.
    int nextFreeBuffer = 0;

    /// Flag that specifies if one-time initialization needs to take place.
    bool isInitialized = false;

    /// Mutex used to ensure concurrency control over temporary buffer allocation and deallocation.
    std::mutex allocationMutex;

  private:

    TemporaryBufferData(void) = default;

    TemporaryBufferData(const TemporaryBufferData& other) = delete;

  public:

    /// Returns a reference to the singleton instance of this class.
    /// @return Reference to the singleton instance.
    static TemporaryBufferData& GetInstance(void)
    {
      static TemporaryBufferData temporaryBufferData;
      return temporaryBufferData;
    }
  };

  TemporaryBufferBase::TemporaryBufferBase(void)
  {
    TemporaryBufferData& data = TemporaryBufferData::GetInstance();
    std::scoped_lock lock(data.allocationMutex);

    if (false == data.isInitialized)
    {
      for (int i = 0; i < _countof(data.freeBuffers); ++i)
        data.freeBuffers[i] = &data.staticBuffers[TemporaryBufferBase::kBytesPerBuffer * i];

      data.nextFreeBuffer = _countof(data.freeBuffers) - 1;
      data.isInitialized = true;
    }

    if (data.nextFreeBuffer < 0)
    {
      buffer = new uint8_t[TemporaryBufferBase::kBytesPerBuffer];
      isHeapAllocated = true;
    }
    else
    {
      buffer = data.freeBuffers[data.nextFreeBuffer];
      data.nextFreeBuffer -= 1;
      isHeapAllocated = false;
    }
  }

  TemporaryBufferBase::~TemporaryBufferBase(void)
  {
    // Buffer pointer can be `nullptr` if the move constructor is used because it initializes the
    // buffer pointer to `nullptr` before doing assignment. Afterwards the temporary object that is
    // consumed has `nullptr` as its buffer pointer when it is destroyed.

    if (nullptr != buffer)
    {
      TemporaryBufferData& data = TemporaryBufferData::GetInstance();

      if (true == isHeapAllocated)
      {
        delete[] buffer;
      }
      else
      {
        std::scoped_lock lock(data.allocationMutex);

        data.nextFreeBuffer += 1;
        data.freeBuffers[data.nextFreeBuffer] = buffer;
      }
    }
  }
} // namespace Xidi
