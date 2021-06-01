// Copyright 2008 Willow Garage, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Willow Garage, Inc. nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/**
 * \file
 *
 * The MessageCollection class
 *
 * \author Bhaskara Marthi
 */

#ifndef WAREHOUSE_ROS_MESSAGE_COLLECTION_H
#define WAREHOUSE_ROS_MESSAGE_COLLECTION_H

#include <warehouse_ros/query_results.h>

namespace warehouse_ros
{
class MessageCollectionHelper
{
public:
  virtual bool initialize(const std::string& datatype, const std::string& md5) = 0;
  virtual void insert(char* msg, size_t msg_size, Metadata::ConstPtr metadata) = 0;
  virtual ResultIteratorHelper::Ptr query(Query::ConstPtr query, const std::string& sort_by = "",
                                          bool ascending = true) const = 0;
  virtual unsigned removeMessages(Query::ConstPtr query) = 0;
  virtual void modifyMetadata(Query::ConstPtr q, Metadata::ConstPtr m) = 0;
  virtual unsigned count() = 0;
  virtual Query::Ptr createQuery() const = 0;
  virtual Metadata::Ptr createMetadata() const = 0;
  virtual std::string collectionName() const = 0;

  typedef boost::shared_ptr<MessageCollectionHelper> Ptr;
};

/// Represents a collection of ROS Messages stored in a database.
/// Each stored message in the db has a unique id, a creation time, and
/// optional additional metadata stored as a dictionary
template <class M>
class MessageCollection
{
public:
  /// \brief Will connect to given database and collection.  Collection is
  /// created if it doesn't exist.
  MessageCollection(MessageCollectionHelper::Ptr collection);

  /// \brief Copy constructor
  MessageCollection(const MessageCollection& rhs);

  /// \brief Destructor
  ~MessageCollection();

  MessageCollection& operator=(const MessageCollection& other);

  /// \brief Insert a ROS message, together with some optional metadata,
  /// into the db
  /// \throws mongo::DBException if unable to insert
  /// \param metadata Metadata to insert.  Note that a unique id field
  /// _id and a field creation_time will be autogenerated for all messages.
  void insert(const M& msg, Metadata::Ptr metadata);

  /// \retval Iterator range over matching messages
  /// \param query A query object
  /// \param metadata_only If this is true, only retrieve the metadata
  /// (returned message objects will just be default constructed)
  typename QueryResults<M>::range_t query(Query::ConstPtr query, bool metadata_only = false,
                                          const std::string& sort_by = "", bool ascending = true) const;

  /// \brief Convenience function that calls queryResult and
  /// puts the results into a vector
  std::vector<typename MessageWithMetadata<M>::ConstPtr> queryList(Query::ConstPtr query, bool metadata_only = false,
                                                                   const std::string& sort_by = "",
                                                                   bool ascending = true) const;

  /// \brief Convenience function that returns a single matching result
  /// for the query
  /// \throws NoMatchingMessageException
  typename MessageWithMetadata<M>::ConstPtr findOne(Query::ConstPtr query, bool metadata_only = false) const;

  /// \brief Remove messages matching query
  unsigned removeMessages(Query::ConstPtr query);

  /// \brief Modify metadata
  /// Find message matching \a q and update its metadata using \a m
  /// In other words, overwrite keys in the message using \a m, but
  /// keep keys that don't occur in \a m.
  void modifyMetadata(Query::ConstPtr q, Metadata::ConstPtr m);

  /// \brief Count messages in collection
  unsigned count();

  /// \brief Check if the md5 sum of the messages previously stored in
  /// the database matches that of the compiled message
  bool md5SumMatches() const;

  Query::Ptr createQuery() const;

  Metadata::Ptr createMetadata() const;

  typedef boost::shared_ptr<MessageCollection<M> > Ptr;
  typedef M message_type;

private:
  MessageCollectionHelper::Ptr collection_;
  bool md5sum_matches_;
};

}  // namespace warehouse_ros

#include "impl/message_collection_impl.hpp"

#endif  // include guard