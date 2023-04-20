# MTS Database Component

Contains both the configuration and the live data for the Middleware Task Scheduler component


# Technical Tables

## BACKEND_REGISTRATION

This table is used to coordinate the multiple instances of the MTS_Spooler.

The MTS_Spooler have the role of scanning the existing data tables to find tasks that are scheduled in the past and not yet triggered. If done naively, they would all attempt to pick up the same tasks leading to high contention. And unfortunately, the `skip locked` clause is a high-cost one in Oracle.

Therefore, we use an alternative strategy: we dynamically partition the tasks in as many partitions as there are instances currently scanning the tables. This is done with a simple scheme:

  - each instance is given a position P in [0, N) where N is the total number of instances
  - each instance only selects tasks such that `task_id % N == P`

This table is used by each instance to deduce which P and N numbers to use. Each instance reads the table (filtering on the `BACKEND_TYPE` column) and builds a sorted list: P is its position in the list and N the list length.

The `REGISTRATION_TIME` column is used to remove instances that have been inactive for a while, as keeping them would lead to some dynamic partitions never being scanned.


## CACHE_VERSION

For efficiency and ease use of reasons, the backends cache the different parts of the configuration. When reading a part of the configuration, they store the current version. Then, at each transaction, they compare the version they have with the version in DB, and if it differs they reload it.


# Configuration Tables

## TASK_CONF_CUSTOM

This table is used to customize the use of criteria for some task types in a given family.

In theory, a family is supposed to contain an homogeneous set of task types, however sometimes a given task type shares everything but a criterion with its siblings:

  - 'D' can be used to *disable* (and forbid) the presence of a criterion
  - 'O' can be used to make the presence of a criterion *optional*


## TASK_CONF_DICT

This table is used to remap the criterion name; it is only of use for the AWRC task types of the QP family.

When originally created, the AWRC task types used the RECLOC and PNR_CREATION_DATE names for what was decided should be IDENTIFIER and CREATION_DATE criteria. In the old EDI interface, for those task types, we use those old names for backward compatibility reasons.

The `DISABLED` column has been made obsolete by the new `TASK_CONF_CUSTOM` table; it is no longer read.


## TASK_CONF_FAMILY_DESC

This table is the entry point of a new family (all family code foreign keys reference it).


## TASK_CONF_FAMILY_MODEL

This table contains the list of criteria of each family.


## TASK_CONF_PARTITION_SCHEME

This table contains the list of partition schemes.

The partition schemes are defined by classes in the code. This table does not configure them, and introducing a new row in this table does not auto-magically introduce a new scheme in the code. Instead, the only role of this table is to prevent the `TASK_CONF_FAMILY_DESC` table to accidentally mention a non-existing scheme when a family is inserted or updated.


## TASK_CONF_SEARCH_ALLOWED

This table contains the list of searches that may be executed on a given family.

Searching in a large table requires either a long wait or a suitably crafted index. In order to protect ourselves from clients requesting long queries that would overwhelm the database and block the MTS_Reader backends, we instead maintain a white-list of searches that have been validated to work reliably.

This table therefore contains, per family, the list of criteria necessary for the use of a given index. A search is allowed provided that it contains all the criteria requested by at least one index; if multiple indices are eligible, the one with the more criteria should be selected.


## TASK_CONF_TYPE_DESC

This table contains the list of task types available in each family.

For each task types, it contains the delivery parameters specific to this task type.


## TASK_MIGRATION_STATUS

This table contains the migration status of each family.

A family absent from this table is considered not to be migrated, at all.

Otherwise, the migration process involves 5 steps:

 - 0: Old master
 - 1: Old master, sync'ing new
 - 2: Old master, sync'ing new and reporting discrepancies
 - 3: New master, sync'ing old
 - 4: New master

When new families are introduced, they should directly use status 4.


## TASK_SUSPENDED

This table contains the list of tasks suspended by a given suspension, as well as their execution timestamp.

It is used to resume the tasks that were suspended by a given suspension.

Their execution timestamp is used to remove their references here once purged from their table.

Note: this design is sub-par, notably because of the lack of partitioning. My recommendation would be, instead, to introduce a new `SUSPENSION_ID` column in each `TASK_DATA_XXX` table, initially `null`, and with a foreign key on the `TASK_SUSPENSION` table. It should be indexed properly (`PARTITION_CODE`, `SUSPENSION_ID`, `EXECUTION_TIME`) and this index would suffice in awakening any remaining suspended task when removing a suspension.


## TASK_SUSPENSION

This table contains the list of active suspensions.


## TASK_SUSPENSION_CRITERIA

This table contains the criteria (and their value) associated to each suspension.

If a suspension has no criterion specified, then it concerns all tasks.


## TASK_SUSPENSION_TYPES

This table contains the task types associated to each suspension.

If a suspension has no task type specified, then it concerns the whole family.


# Data Tables

## TASK_DATA_XXX

This table contains the list of tasks of a given family.

All such tables share a common prefix, columns that are present in all of them, and then a specific set of columns starting by the `FUNC_` prefix which contain the value of the criteria of the task.


## TASK_DATA_XXX_SWAP

This table is used to purge its twin.

This table is a mirror of its twim: same columns, same indexes and constraints. The only notably difference is that unlike its twin it is not partitioned. It is used as part of the purge process, via the `exchange partition` feature of Oracle.


# Old Model: Configuration Tables

The following tables are used to configure the old model.

## DELIVER_ID_TASK_TYPE

This table maps the task type to the group of MTS_Deliver backends in charge of it. In practice, all task types are mapped to the same group.


## GUARANTEE_PROCESSING

This table is supposed to be used to program different schedule delays and retry policies, ... in practice it is unused and obsolete.


## JOB_TYPE

This table contains the configuration of a given task type.


## JOB_TYPE_CRITERIA

This table contains the list of criteria associated to each task type, and whether each criteria is part of the functional key or not.


## TRIGGER_DELIVERY

Unclear.


## TRIGGER_ID_TASK_TYPE

This table maps the task type to the group of MTS_Trigger backends in charge of it. In practice, all task types are mapped to the same group.


# Old Model: Data Tables

The following tables are used to operate the old model.

## JOB_XXX

This table contains the list of tasks for the XXX task type.

Note: the `CONTENT`, `CONTENT_TYPE` and `CONTENT_LENGTH` columns are obsolete. Some others may also be obsolete.


## JOB_CRITERIA_VALUE_XXX

This table contains the list of criteria associated to each task type (in a one to many relationship).


## MUTEX

This table is used to guarantee unicity.

No two tasks with the same set of Key Functional Criteria should ever exist at any point in time. To do so, a hash code is computed for each task based on the Key Functional Criteria plus some other information (task type, ...) and this hash code is inserted in the `MUTEX` table prior to any action to prevent concurrent execution for a same task.


## SUSPENDED_DATA

Originally used to power the Suspend usecase. Due to performance reasons this usecase was removed from the old model, and therefore this table is obsolete.


## TEMP_ID_DATE

Unclear.


# Migration Tables

The following tables were used to migrate the QP family from the old model to the new model. The custom PL/SQL OSR used for this was lost since...

## TMP_TASK_COPY_ERROR

Unclear.


## TMP_TASK_TO_COPY

Unclear.
