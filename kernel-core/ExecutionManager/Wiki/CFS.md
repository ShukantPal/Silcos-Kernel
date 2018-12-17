# Introduction

CFS (or Completely Fair Scheduling) is a scheduler-class that **incorporates task execution by keeping them in a red-black tree to sort them by time-based priority**. The task with the highest dynamic priority will be given the chance to use the CPU. Various methods are used to modify the dynamic priority of tasks like changing the static priority or increasing I/O activity.