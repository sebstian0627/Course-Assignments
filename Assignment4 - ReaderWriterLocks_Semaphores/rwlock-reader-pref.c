#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
  rw->readers=0;
  rw->writers=0;
  sem_init(&rw->read_lock, 0, 1);
  sem_init(&rw->resource_lock, 0, 1);
  sem_init(&rw->write_lock, 0, 1);
  sem_init(&rw->readTry, 0, 1);
}

void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  sem_wait(&rw->lock);
  rw->readers++;
  if(rw->readers==1){
    sem_wait(&rw->resource_lock);

  }
  sem_post(&rw->read_lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  sem_wait(&rw->read_lock);
  rw->readers--;
  if (rw->readers == 0)
  sem_post(&rw->resource_lock); // last reader releases writelock
  sem_post(&rw->read_lock);
}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  sem_wait(&rw->resource_lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  sem_post(&rw->resource_lock);
}
