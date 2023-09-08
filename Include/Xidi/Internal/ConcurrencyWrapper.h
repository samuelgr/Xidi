/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file ConcurrencyWrapper.h
 *   Utility template for adding concurrency-safe operations to data.
 **************************************************************************************************/

#pragma once

#include <condition_variable>
#include <shared_mutex>
#include <stop_token>

namespace Xidi
{
  /// Wraps data in a way that is concurrency-safe following a single-producer multiple-consumer
  /// threading model.
  /// @tparam DataType Underlying wrapped data type.
  template <typename DataType> class ConcurrencyWrapper
  {
  public:

    /// Retrieves and returns the stored data in a concurrency-safe way.
    /// @return Underlying wrapped data.
    inline DataType Get(void)
    {
      std::shared_lock lock(mutex);
      return data;
    }

    /// Writes to the stored data in a concurrency-safe way.
    /// @param [in] newData New data to be stored.
    inline void Set(const DataType& newData)
    {
      std::unique_lock lock(mutex);
      data = newData;
    }

    /// Updates the stored data in a concurrency-safe way and notifies all waiting threads of the
    /// change. Operations are conditional on the new data being different than the currently-stored
    /// data.
    /// @param [in] newData New data to be stored.
    /// @return `true` if the new data differ from the old and hence an update was performed,
    /// `false` otherwise.
    inline bool Update(const DataType& newData)
    {
      // This unguarded read is safe because by design only one thread, the one that produces
      // updated data, ever invokes this method. All other threads use guarded reads.
      if (newData != data)
      {
        Set(newData);
        updateNotifier.notify_all();
        return true;
      }

      return false;
    }

    /// Waits for the stored data to be updated.
    /// This function is fully concurrency-safe. If needed, the caller can interrupt the wait using
    /// a stop token.
    /// @param [in,out] externalData On input, used to identify the last-known data for the calling
    /// thread. On output, filled in with the updated data.
    /// @param [in] stopToken Token that allows the wait to be interrupted.
    /// @return `true` if the wait succeeded and an update occurred, `false` if no updates were made
    /// due to invalid parameter or interrupted wait.
    inline bool WaitForUpdate(DataType& externalData, std::stop_token stopToken)
    {
      std::shared_lock lock(mutex);

      updateNotifier.wait(
          lock,
          stopToken,
          [this, &externalData]() -> bool
          {
            return (data != externalData);
          });

      if (stopToken.stop_requested()) return false;

      externalData = data;
      return true;
    }

  private:

    /// Wrapped data.
    DataType data;

    /// Condition variable used to wait for updates to the underlying wrapped data.
    std::condition_variable_any updateNotifier;

    /// Mutex for protecting against concurrent accesses to the underlying wrapped data.
    std::shared_mutex mutex;
  };
} // namespace Xidi
