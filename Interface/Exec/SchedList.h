/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef EXEC_SCHED_LIST_H
#define EXEC_SCHED_LIST_H

#include <Exec/KTask.h>
#include <Types.h>
#include <Util/LinkedList.h>

/* Scheduling Entities - All ExecTp's except ProcessGroupTp. */
typedef struct KTask SchedEntity;

/* Helper macros */
#define OwnerList(Ex) ((struct SchedulerList*) Ex -> OwnerList)
#define SchedSize(Sl) (Sl -> Size)
#define StartEnt(Sl) (Sl -> Head)
#define EndEnt(Sl) (Sl -> Tail)

#define HeadEx(Ex) (OwnerList(Ex) -> Start)
#define TailEx(Ex) (OwnerList(Ex) -> End)

#define AdoptSched(Ex, Sl) (Ex -> OwnerList = Sl)

typedef LinkedList ExecList;

#define AddEntity(List, Entity) KRUNNABLE *e = Entity; e->OwnerList=List; AddElement((LIST_ELEMENT*) Entity,(LinkedList*) List);
#define RemoveEntity(Entity) RemoveElement((LIST_ELEMENT*) Entity, (LinkedList*) ((KRUNNABLE *)Entity)->OwnerList)

#endif /* Exec/SchedList.h */
