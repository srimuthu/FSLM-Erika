#-------------------------------------------------------------------------------
# File Name : FlexTool.py
# Purpose   : Utility tool for FSLM implementation in Erika Enterprise
#             1. Determine priorities from OIL file
#             2. Receive user input for spin-lock priorities
#             3. Add spin-lock priorities to Erika application
#             4. Modify stack to create dual shared stack
# Author    : Sri Muthu Narayanan Balasubramanian
# Created   : Feb 2017
# Copyright : 
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Imports
#-------------------------------------------------------------------------------
import sys, os , time
import math
import re
import copy
from prettytable import PrettyTable

#-------------------------------------------------------------------------------
# Visualization flags
#-------------------------------------------------------------------------------
DEBUG   = False
UI      = True

#-------------------------------------------------------------------------------
# File paths
#-------------------------------------------------------------------------------
# Source file paths
# If the source files exist in a different directory
# prefix SOURCE_NAME_TEMPLATE with the path to the variable
# Ensure that the path does not contain the sub-string "XXX"
SOURCE_NAME_TEMPLATE    = "cpuXXX_main.c"
EECFG_NAME_TEMPLATE     = "Debug/cpuXXX/eecfg.c"
OIL_PATH                = "conf.oil"

#-------------------------------------------------------------------------------
# String definitions
#-------------------------------------------------------------------------------

# OIL file strings
CPU_DATA        = "CPU_DATA"
CPU_NAME        = "ID"

TASK_DATA       = "\tTASK "
TASK_CPU        = "CPU_ID"
TASK_PRIORITY   = "PRIORITY"
TASK_RESOURCE   = "RESOURCE"

# CPU application file strings
SPIN_PRIO       = "EE_th_spin_prio"
GLOBAL_TASK_ID  = "GlobalTaskID"
RES_TASK        = "EE_resource_task"

# eecfg.c file strings
THREAD_TOS      = "EE_hal_thread_tos"
SYSTEM_TOS      = "EE_nios2_system_tos"

# Warning strings
WARN_STR        = "\t\n/*!!!THE STACK INFORMATION HAS BEEN MODIFIED BY FLEXTOOL!!*/\n"

#-------------------------------------------------------------------------------
# Class FlexTool
#-------------------------------------------------------------------------------
class FlexTool:
    """
        FlexTool Class - Utility tool for FSLM implementation in Erika Enterprise 

        Comments    :   
                        1. Determine priorities from OIL file

                        2. Receive user input for spin-lock priorities

                        3. Add spin-lock priorities to Erika application

                        4. Modify configuration files to create dual shared stack

    """

    def __init__(self, oilPath):
        """  
            Constructor for FlexTool class
        """

        self.__initSuccess = False

        # Check if the path is valid:
        if not os.path.isfile(oilPath):
            sys.exit("OIL file in the specified path does not exist")

        # Initialize pvt data structures
        self.__fileBuffer = ""
        self.__fileFullText = ""
        self.__oilPath = oilPath
        # __cpuInfo:
        # { cpuID (int) : cpuName (str) }
        self.__cpuInfo = {}
        # __taskInfo:
        # taskID (int) : [  [0] taskName (str),
        #                   [1] cpuID (int),
        #                   [2] cpuName (str),
        #                   [3] taskPriority (int),
        #                   [4] resourceBool (bool),
        #                   [5] resources (list) = [resourceName (str)] ]
        self.__taskInfo = {}
        self.__resourceInfo = {}
        self.__task_block = []

        # FlexSpin tool variables
        self.__tasks2cores = {}
        self.__tasks2res = {}
        self.__tasks2prio = {}
        self.__resScope = {}
        self.__tasks2globRes = {}
        self.__cpPrio = {}
        self.__cpHatPrio = {}
        self.__hpPrio = {}
        self.__userPrio = {}
        self.__spinPrio = {}
        self.__tasks2stack = {}
        self.__isStackEditRequired = {}
        # Set initialization successful
        self.__initSuccess = True

    def __parseCpuInfo(self):
        """
            Extract CPU information from OIL file

            Called by   :   parseOilFile()

            Comments    :   
                            Iterate through the OIL file and save CPU info to __cpuInfo dict
                            __cpuInfo:
                            { cpuID (int) : cpuName (str) }

        """

        if self.__fileBuffer == "":
            sys.exit("OIL file is empty")

        # Local variables
        cpu_info_start = False
        cpu_count = -1

        # Iterate through lines
        for line in self.__fileBuffer:
            if line.find(CPU_DATA) != -1:
                cpu_info_start = True
                continue
            if cpu_info_start:
                if line.find(CPU_NAME) != -1:
                    cpu_info_start = False
                    cpu_count += 1
                    try:
                        self.__cpuInfo[cpu_count] = re.search('"(.+?)"', line).group(1)
                    except:
                        sys.exit("OIL file cpu data error")

    def __createTaskData(self, task_counter):
        """
            Create Task information data structure from OIL file

            Comments:
                        The __taskInfo dict consists of:
                        
                        taskID (int) : [  [0] taskName (str),

                                          [1] cpuID (int),

                                          [2] cpuName (str),

                                          [3] taskPriority (int),

                                          [4] resourceBool (bool),

                                          [5] resources (list) = [resourceName (str)] ]

        """

        if not self.__task_block:
            sys.exit("Task  block improperly defined in OIL file")

        self.__taskInfo[task_counter] = []

        for line in self.__task_block:

            if line.find(TASK_DATA) != -1:
                self.__taskInfo[task_counter].append(str(re.search('TASK (.+?) ', line).group(1)))

            if line.find(TASK_CPU) != -1:
                cpu = re.search('"(.+?)"', line).group(1)
                self.__taskInfo[task_counter].append(int(list(self.__cpuInfo.keys())[list(self.__cpuInfo.values()).index(cpu)]))
                self.__taskInfo[task_counter].append(str(cpu))

            if line.find(TASK_PRIORITY) != -1:
                prio = re.search('PRIORITY =(.+?);', line).group(1)
                self.__taskInfo[task_counter].append(int(prio))

            if line.find(TASK_RESOURCE) != -1:
                if len(self.__taskInfo[task_counter]) <= 4:
                    self.__taskInfo[task_counter].append(True)
                    self.__taskInfo[task_counter].append([])
                    self.__taskInfo[task_counter][5].append(str(re.search('RESOURCE =(.+?);', line).group(1)).strip())
                else:
                    self.__taskInfo[task_counter][5].append(str(re.search('RESOURCE =(.+?);', line).group(1)).strip())


        if len(self.__taskInfo[task_counter]) < 5:
            self.__taskInfo[task_counter].append(False)
            self.__taskInfo[task_counter].append([])

    def __parseTaskInfo(self):
        """Extract task information from OIL file"""

        if not self.__cpuInfo:
            sys.exit("No CPU information found in OIL file")

        task_info_start = False
        task_counter = -1
        brace_counter = 0

        for line in self.__fileBuffer:
            if line.find(TASK_DATA) != -1 and not task_info_start:
                task_info_start = True
                task_counter += 1
                self.__task_block = []

            if task_info_start:
                if line.find('{') != -1:
                    brace_counter += 1
                if line.find('}') != -1:
                    brace_counter -= 1
                self.__task_block.append(line)
                if brace_counter == 0:
                    task_info_start = False
                    self.__createTaskData(task_counter)

    def __createResourceList(self):
        """
            Create Resouce list data structure from OIL file

            Comments:
                        __resourceInfo dict consists of

                        {resID (int) :   resource name (str)}

        """

        if not self.__taskInfo:
            sys.exit("No task information available from OIL file")

        res_list = []
        for task in self.__taskInfo:
            for item in self.__taskInfo[task][5]:
                res_list.append(item)

        res_list = sorted(list(set(res_list)), key=str.lower)
        for i in range(len(res_list)):
            self.__resourceInfo[i] = res_list[i]

    def parseOilFile(self):
        """
            Parse and extract information from OIL file

            Returns     :   
                            self.__cpuInfo, self.__taskInfo, self.__resourceInfo

            Calls       :   
                            __parseCpuInfo(), __parseTaskInfo(), __createResourceList()

            Called by   :   
                            main() using FlexTool object

        """

        if not self.__initSuccess:
            sys.exit("flexSpin class not initialized properly")

        with open(self.__oilPath, 'r') as f:
            self.__fileBuffer = f.readlines()

        with open(self.__oilPath, 'r') as f:
            self.__fileFullText = f.read()

        self.__parseCpuInfo()
        self.__parseTaskInfo()
        self.__createResourceList()


        if DEBUG:
            print("OIL parser output START: \n")
            print("CPU Information :")
            print(self.__cpuInfo)
            print("\n Task Information:")
            print(self.__taskInfo)
            print("\n Resource Information:")
            print(self.__resourceInfo)
            print("\n OIL parser output END \n")

        return self.__cpuInfo, self.__taskInfo, self.__resourceInfo

#############################################################################################
# END OF OIL PARSER
#############################################################################################

#############################################################################################
# START OF FLEXSPIN TOOL
#############################################################################################

    def initializeFlexSpinToolVars(self):
        """
            Initialize the flexible spin-lock priority tool related variables

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            Uses variables __cpuInfo, __taskInfo, __resourceInfo to construct mapping between tasks, cores and resources

        """
        for task in self.__taskInfo:
            # task2cores mapping
            self.__tasks2cores[task] = self.__taskInfo[task][1] # cpuID

            # task2prio mapping
            self.__tasks2prio[task] = self.__taskInfo[task][3] # task priority

            # task2res mapping
            self.__tasks2res[task] = []
            if self.__taskInfo[task][4]: # resource bool true
                for resource in self.__taskInfo[task][5]:
                    self.__tasks2res[task].append(int(
                        list(self.__resourceInfo.keys())
                        [list(self.__resourceInfo.values()).index(resource)]
                    ))

    def __findGlobalResources(self):
        """
            Identify global resources (classify local and global resources)
        """

        # Initialize with all resources set as local (bool False)
        for resource in self.__resourceInfo:
            self.__resScope[resource] = False

        for resource in self.__resourceInfo:
            cores_using_resource = []
            for task in self.__tasks2res:
                if resource in self.__tasks2res[task]:
                    cores_using_resource.append(self.__tasks2cores[task])

            cores_using_resource = list(set(cores_using_resource))
            if len(cores_using_resource) > 1:
                self.__resScope[resource] = True

        # Update tasks2global resources mapping
        self.__tasks2globRes = copy.deepcopy(self.__tasks2res)
        for task in self.__tasks2res:
            for resource in self.__tasks2res[task]:
                if not self.__resScope[resource]:
                    self.__tasks2globRes[task].remove(resource)

    def __reducePriorities(self):
        """
            Transform priority levels within a core to consecutive values starting with 0 (lowest)
        """

        for core in self.__cpuInfo:
            prio_in_core = []
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    prio_in_core.append(self.__tasks2prio[task])
            prio_in_core.sort()
            prio_reduced = {}
            for prio in prio_in_core:
                prio_reduced[prio] = prio_in_core.index(prio)+1
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    for prio in prio_reduced:
                        if self.__tasks2prio[task] == prio:
                            self.__tasks2prio[task] = prio_reduced[prio]


    def __displayParams(self):
        """
            Output task, resource and CPU info to the console
        """

        print("Mapping of cores and tasks ")
        t = PrettyTable(['TaskID','Task','CoreID','Core'])
        for task in self.__tasks2cores:
            t.add_row([task,
                self.__taskInfo[task][0],
                self.__tasks2cores[task],
                self.__cpuInfo[self.__tasks2cores[task]]])
        print(t)

        print("\nMapping of tasks and resources ")
        for task in self.__tasks2res:
            output = str(self.__taskInfo[task][0])+"  :\t"
            for res in self.__tasks2res[task]:
                output += str(self.__resourceInfo[res])+"\t"
            print(output)

        print("\nList of global resources ")
        for res in self.__resScope:
            if self.__resScope[res]:
                print(self.__resourceInfo[res])

        print("\nMapping of cores and resources ")
        for core in self.__cpuInfo:
            output = str(self.__cpuInfo[core])+" :\t"
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    for res in self.__tasks2res[task]:
                        if output.find(self.__resourceInfo[res]) == -1:
                            output += str(self.__resourceInfo[res])+"\t"
            print(output)

        print("\nDerived priorities ")
        head1 = [" "]
        head2 = [str(self.__cpuInfo[cpu]) for cpu in self.__cpuInfo]
        t = PrettyTable(head1+head2)
        output = ["hp"]
        for item in self.__hpPrio:
            output.append(str(self.__hpPrio[item]))
        t.add_row(output)
        output = ["cp^"]
        for item in self.__cpHatPrio:
            output.append(str(self.__cpHatPrio[item]))
        t.add_row(output)
        output = ["cp"]
        for item in self.__cpPrio:
            output.append(str(self.__cpPrio[item]))
        t.add_row(output)
        print(t)

    def __calculateSpinPriorities(self):
        """
            Convert user input spin-priorities to HEX
        """
        for core in self.__userPrio:
            self.__spinPrio[core] = int(math.pow(2,self.__userPrio[core]-1))

    def calculatePriorities(self):
        """
            Calculate CP, CP hat and HP priorities

            Calls       :   
                            __findGlobalResources()

                            __reducePriorities()

                            __displayParams()

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            1. Identify global variables

                            2. Transform priority levels within a core to consecutive values starting with 0 (lowest)

                            3. Use available data to find CP, CP hat and HP for every core

        """

        self.__findGlobalResources()
        self.__reducePriorities()
        # Calculate CP
        for core in self.__cpuInfo:
            max_prio = 0
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    if self.__tasks2globRes[task]: # this task uses a global resource
                        if self.__tasks2prio[task] > max_prio:
                            max_prio = self.__tasks2prio[task]
            self.__cpPrio[core] = max_prio



        # Calculate CP Hat
        for core in self.__cpuInfo:
            max_prio = 0
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    if self.__tasks2res[task]: # this task uses a global resource
                        if self.__tasks2prio[task] > max_prio:
                            max_prio = self.__tasks2prio[task]
            self.__cpHatPrio[core] = max_prio


        # Calculate HP
        for core in self.__cpuInfo:
            max_prio = 0
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    if self.__tasks2prio[task] > max_prio:
                        max_prio = self.__tasks2prio[task]
            self.__hpPrio[core] = max_prio

        if DEBUG:
            print("initialize output START: \n")
            print("task2cores :")
            print(self.__tasks2cores)
            print("\ntask2prio :")
            print(self.__tasks2prio)
            print("\ntask2res:")
            print(self.__tasks2res)
            print("\ntask2globalres:")
            print(self.__tasks2globRes)
            print("\nresource Scope:")
            print(self.__resScope)
            print("CP priority : ")
            print(self.__cpPrio)
            print("CP Hat priority : ")
            print(self.__cpHatPrio)
            print("HP priority : ")
            print(self.__hpPrio)
            print("\ninitialize parser output END \n")

        if UI:
            self.__displayParams()

    def __calculateStackAllocation(self):
        """
            Group tasks on a core to 2 stacks based on spin-lock priority
        """

        for core in self.__cpuInfo:
            stack_edit_counter = 0
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    if self.__tasks2prio[task] > self.__userPrio[core]:
                        stack_edit_counter += 1
                        self.__tasks2stack[task] = [self.__taskInfo[task][0], "STACK2"]
                    else:
                        self.__tasks2stack[task] = [self.__taskInfo[task][0], "STACK1"]

            if stack_edit_counter > 1:
                self.__isStackEditRequired[core] = True
            else:
                self.__isStackEditRequired[core] = False
        if UI:
            print("\nSpin priorities ")
            head1 = [" "]
            head2 = [str(self.__cpuInfo[cpu]) for cpu in self.__cpuInfo]
            t = PrettyTable(head1 + head2)
            output = ["sp"]
            for item in self.__spinPrio:
                output.append(str(self.__spinPrio[item]))
            t.add_row(output)
            print(t)

            print("\nStack allocation ")
            t = PrettyTable(['CPU', 'Task', 'Stack'])
            for task in self.__tasks2stack:
                t.add_row([str(self.__tasks2cores[task])]+ self.__tasks2stack[task])
            print(t)

    def getUserInput(self):
        """
            Get input from the user

            Calls       :   
                            __calculateSpinPriorities()

                            __calculateStackAllocation()

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            1. Prompt user for spin-lock priority per core

                            2. Convert priorities to HEX

                            3. Determine stack allocation of tasks into dual stacks on each core 

        """
        for i in range(len(self.__cpuInfo)):
            valid = False
            while not valid:
                input_msg = "Enter a priority between {0} and {1} for {2}: ".format(
                    self.__cpPrio[i],
                    self.__hpPrio[i],
                    self.__cpuInfo[i])
                self.__userPrio[i] = int(input(input_msg))
                if ((self.__userPrio[i] >= self.__cpPrio[i])
                    and (self.__userPrio[i] <= self.__hpPrio[i])):
                    valid = True
                else:
                    print("Invalid input . . Enter again")

        self.__calculateSpinPriorities()
        self.__calculateStackAllocation()

    def returnFlexSpinInfo(self):
        """
            Return __spinPrio, __tasks2stack, __cpuInfo, __tasks2cores 
        """
        return self.__spinPrio, self.__tasks2stack, self.__cpuInfo, self.__tasks2cores

#############################################################################################
# END OF FLEXSPIN TOOL
#############################################################################################

#############################################################################################
# START OF SOURCE FILES UPDATE
#############################################################################################

    def updateSourceFiles(self):

        """
            Update the application source files based on user input for spin-lock priority

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            Update "cpuXX_main.c" files for all cores

                            1. update EE_th_spin_prio

                            2. update GlobalTaskID

                            3. update EE_resource_task

                            Throws error if the above variables are not found in the file

                            Also throws error if the file "cpuXX_main.c" is not found (XX = cpu ID (integer))
                            

        """

        for cpu in self.__cpuInfo:
            file = SOURCE_NAME_TEMPLATE.replace("XXX",str(cpu))
            # Check if the path is valid:
            if not os.path.isfile(file):
                sys.exit("Source file in the specified path does not exist")

        for cpu in self.__cpuInfo:
            fileName = SOURCE_NAME_TEMPLATE.replace("XXX", str(cpu))

            if UI:
                print("Updating file: "+fileName)

            with open(fileName, 'r') as file:
                data = file.readlines()

            # line_id =[ "EE_th_spin_prio", "GlobalTaskID"]
            indices = [-1, -1, -1]

            if data:
                for line in data:
                    if line.find(SPIN_PRIO) != -1:
                        indices[0] = data.index(line)
                    if line.find(GLOBAL_TASK_ID) != -1:
                        indices[1] = data.index(line)
                    if line.find(RES_TASK) != -1:
                        indices[2] = data.index(line)
            else:
                sys.exit("No content present in source file")

            if indices[0] != -1:
                spin_text = "const int "+SPIN_PRIO+"[] = {"
                for task in self.__tasks2cores:
                    if self.__tasks2cores[task] == cpu:
                        spin_text += "0x"+str(self.__spinPrio[cpu])+","
                spin_text = spin_text[:-1]
                spin_text += "};\n"
                data[indices[0]] = spin_text

            else:
                print("NOT found EE_th_spin_prio - Application may not work as intended")

            if indices[1] != -1:
                glob_text = "const int " + GLOBAL_TASK_ID + "[] = {"
                for task in self.__tasks2cores:
                    if self.__tasks2cores[task] == cpu:
                        glob_text += str(task) + ","
                glob_text = glob_text[:-1]
                glob_text += "};\n"
                data[indices[1]] = glob_text

            else:
                print("NOT found GlobalTaskID - Application may not work as intended")

            if indices[2] != -1:
                glob_text = "EE_TID " + RES_TASK + "[] = {"
                for res in self.__resourceInfo:
                    glob_text += "-1" + ","
                glob_text = glob_text[:-1]
                glob_text += "};\n"
                data[indices[2]] = glob_text

            else:
                print("NOT found EE_resource_task - Application may not work as intended")

            with open(fileName, 'w') as file:
                file.writelines(data)

    def updateOilFile(self):

        """
            Modifies task stack info in OIL file to accommodate dual stack

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            Update "conf.OIL" file with task STACK attribute

                            1. SHARED for tasks with priorities upto spin-lock priority
                            2. PRIVATE for other tasks (later modified to shared after code generation by updateCfgFiles() )
                            
                            --------------!!!WARNING!!!--------------

                            Only works for single line STACK attribute.
                            Please specify the STACK attribute in a single line in the OIL file
                            i.e keyword "STACK" and terminator ";" must be on the same line. 

        """

        with open(OIL_PATH, 'r') as file:
            data = file.readlines()

        task_pos = {}

        for task in self.__tasks2stack:
            task_info_start = False
            task_name = ""
            task_counter = -1
            for line in data:
                if line.find(TASK_DATA) != -1 and not task_info_start:
                    task_name = str(re.search('TASK (.+?) ', line).group(1))
                    if task_name == self.__tasks2stack[task][0]:
                        task_info_start = True
                        task_counter += 1
                        task_pos[task] = data.index(line)

        for task in self.__tasks2stack:
            flag = False
            idx = task_pos[task]
            for i in range(idx, idx+100):
                if data[i].find('STACK') != -1:
                    data[i] = ""
                    output  = "\t\tSTACK = "
                    if self.__tasks2stack[task][1] == "STACK1":
                        output += "SHARED;\n"
                    elif self.__tasks2stack[task][1] == "STACK2":
                        output += "PRIVATE{ \tSYS_SIZE = 0x100; \t};\n"
                    data[i] = output
                    break

        with open(OIL_PATH, 'w') as file:
            file.writelines(data)

        self.promptUser()

    def __findBraceBlock(self, data, item):

        """"
            Identify the brace enclosed block containing a given string

            Arguments: 
                        data - file buffer ,

                        item - item to be found (str)

            Returns:
                        Index of brace block start

                        Index of brace block end

                        Data enclosed between in the block containing the (item)            

        """
        if not data:
            sys.exit("No valid data to parse !! ")

        info_start = False
        brace_counter = 0
        block_start_index = -1
        block_end_index = -1
        block_data = []

        for line in data:
            if line.find(item) != -1 and not info_start:
                info_start = True
                block_start_index = data.index(line)
                block_data = []

            if info_start:
                if line.find('{') != -1:
                    brace_counter += 1
                if line.find('}') != -1:
                    brace_counter -= 1
                block_data.append(line)
                if brace_counter == 0:
                    info_start = False
                    block_end_index = block_start_index + len(block_data)
                    return block_start_index, block_end_index, block_data


    def __editThreadTos(self, block_data, cpu):
        """
            Edit "EE_hal_thread_tos" variable in "eecfg.c"

            Arguments:
                        block_data - Part of file buffer from eecfg.c containing EE_hal_thread_tos,

                        cpu - CPU ID (cpu which the eecfg.c file belongs to)

            Returns:
                        Success or Failed (bool),

                        Edited file buffer of eecfg.c containing EE_hal_thread_tos
        """
        if self.__isStackEditRequired[cpu]:
            t2s_counter = 0
            for task in self.__tasks2stack:
                if self.__tasks2cores[task] == cpu and self.__tasks2stack[task][1] == "STACK2":
                    t2s_counter += 1
                    if t2s_counter > 1:
                        for i in range(0,len(block_data)):
                            if block_data[i].find(self.__tasks2stack[task][0]) != -1:
                                blah1 = block_data[i]
                                blah2 = self.__tasks2stack[task][0]
                                txt2replace = block_data[i][:(block_data[i].find("/*"))]
                                txtNew = txt2replace.replace(str(t2s_counter),"1")
                                block_data[i] = block_data[i].replace(txt2replace,txtNew)
            return True, block_data
        else:
            return False, block_data

    def __editSystemTos(self, block_data, cpu):
        """
            Edit "EE_nios2_system_tos" variable in "eecfg.c"

            Arguments:
                        block_data - Part of file buffer from eecfg.c containing EE_nios2_system_tos,

                        cpu - CPU ID (cpu which the eecfg.c file belongs to)

            Returns:
                        Success or Failed (bool),

                        Edited file buffer of eecfg.c containing EE_nios2_system_tos
        """

        if self.__isStackEditRequired[cpu]:
            new_block = []
            new_block.append(WARN_STR)
            # Line 1:
            line1 = block_data[0]
            numInLine1 = str(re.search('EE_nios2_system_tos(.+?) ', line1).group(1))
            line1 = line1.replace(numInLine1,"[2]")
            new_block.append(line1)
            # Line 2:
            new_block.append(block_data[1])
            # Line 3:
            new_block.append(block_data[2].replace(",",""))
            # Line 4:
            new_block.append(block_data[-1])

            return True, new_block

        else:
            return False, block_data



    def __spliceTextToFileBuffer(self, file_buffer, block_data, start_index, end_index):
        """
            Inserts given text into a file at specified location

            Arguments:
                        file_buffer - Full file buffer

                        block_data - Data to be inserted

                        start_index - Start position to insert

                        end_index - End position 

            Returns:
                        splicedList - Full file buffer with block_data inserted
        """

        splicedList = file_buffer[:start_index]
        splicedList = splicedList + block_data
        splicedList = splicedList + file_buffer[end_index:]
        return splicedList

    def updateCfgFiles(self):
        """
            Function to update Erika configuration files "eecfg.c" on all cores to include stack information

            Called by   :   
                            main() using FlexTool object

            Comments    :   
                            For all cores, modifies the "eecfg.c" to have only 2 shared stacks per core as per dual stack configuration

        """

        for cpu in self.__cpuInfo:
            file = EECFG_NAME_TEMPLATE.replace("XXX",str(cpu))
            # Check if the path is valid:
            if not os.path.isfile(file):
                sys.exit("eecfg.c in the specified path does not exist")

            with open(file, 'r') as f:
                data = f.readlines()
            # Find EE_hal_thread_tos
            bsi, bei, bd = self.__findBraceBlock(data, THREAD_TOS)
            isReplaced, bd = self.__editThreadTos(bd, cpu)

            if isReplaced:
                if UI:
                    print("Modifying stack {0} info for cpu{1}".format(THREAD_TOS, cpu))
                data = self.__spliceTextToFileBuffer(data, bd, bsi, bei)

            #Find EE_nios2_system_tos
            bsi, bei, bd = self.__findBraceBlock(data, SYSTEM_TOS)
            isReplaced, bd = self.__editSystemTos(bd, cpu)

            if isReplaced:
                if UI:
                    print("Modifying stack {0} info for cpu{1}".format(SYSTEM_TOS, cpu))
                data = self.__spliceTextToFileBuffer(data, bd, bsi, bei)

            with open(file, 'w') as f:
                f.writelines(data)


    def promptUser(self):
        """
            Prompts user to rebuild Erika.

            Comments: 
                        This prompt is followed by the editing of eecfg.c files for dual stack implementation

        """
        user_input = None
        valid = False
        print("OIL file and source files updated ! Clean and Build Erika !!")
        print("Do not forget to Refresh the project in Eclipse \n")
        while not valid:
            user_input = input("Press y to resume AFTER THE BUILD is complete! : ")
            if user_input == "y" or user_input == "Y":
                valid = True
            else:
                print("Invalid input .. Enter again")

#############################################################################################
# END OF SOURCE FILES UPDATE
#############################################################################################


if __name__ == "__main__":

    flexObj = FlexTool(OIL_PATH)
    flexObj.parseOilFile()
    flexObj.initializeFlexSpinToolVars()
    flexObj.calculatePriorities()
    flexObj.getUserInput()
    flexObj.updateSourceFiles()
    flexObj.updateOilFile()
    flexObj.updateCfgFiles()

    del(flexObj)
