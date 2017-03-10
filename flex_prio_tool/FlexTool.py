import sys, os , time
import math
from flexStrings import *
import re
import copy

DEBUG = True
UI = True

# If the source files exist in a different directory
# prefix SOURCE_NAME_TEMPLATE with the path to the variable
# Ensure that the path does not contain the string "XXX"
SOURCE_NAME_TEMPLATE = "cpuXXX_main.c"
OIL_PATH = "conf.oil"

class FlexTool:

    def __init__(self, oilPath):
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

        # Set initialization successful
        self.__initSuccess = True

    def __parseCpuInfo(self):
        """Extract CPU information from OIL file"""

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
        """Function for parsing the OIL file"""

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
        """Function to initialize the shared private variables"""
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

        print("Mapping of cores and tasks ")
        print("TaskID\t|\tTask\t|\tCoreID\t|\tCore\t|\t")
        print("------\t|\t----\t|\t------\t|\t----\t|\t")
        for task in self.__tasks2cores:
            print("{0}\t\t|\t{1}\t|\t{2}\t\t|\t{3}\t|\t".format(
                task,
                self.__taskInfo[task][0],
                self.__tasks2cores[task],
                self.__cpuInfo[self.__tasks2cores[task]]
            ))

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
        output = "\t\t|\t"
        for cpu in self.__cpuInfo:
            output += str(self.__cpuInfo[cpu])+"|\t"
        print(output)
        output = "hp\t\t|\t"
        for item in self.__hpPrio:
            output += str(self.__hpPrio[item])+"\t|\t"
        print(output)
        output = "cp^\t\t|\t"
        for item in self.__cpHatPrio:
            output += str(self.__cpHatPrio[item]) + "\t|\t"
        print(output)
        output = "cp\t\t|\t"
        for item in self.__cpPrio:
            output += str(self.__cpPrio[item]) + "\t|\t"
        print(output)

    def __calculateSpinPriorities(self):
        for core in self.__userPrio:
            self.__spinPrio[core] = int(math.pow(2,self.__userPrio[core]-1))

    def calculatePriorities(self):
        """Function to calculate the different priority levels"""

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

        for core in self.__cpuInfo:
            for task in self.__tasks2cores:
                if self.__tasks2cores[task] == core:
                    if self.__tasks2prio[task] > self.__userPrio[core]:
                        self.__tasks2stack[task] = [self.__taskInfo[task][0], "PRIVATE"]
                    else:
                        self.__tasks2stack[task] = [self.__taskInfo[task][0], "SHARED"]

        if UI:
            print("\nSpin priorities ")
            output = "\t\t|\t"
            for cpu in self.__cpuInfo:
                output += str(self.__cpuInfo[cpu]) + "|\t"
            print(output)
            output = "sp\t\t|\t"
            for item in self.__spinPrio:
                output += str(self.__spinPrio[item]) + "\t|\t"
            print(output)

            print("\nStack allocation ")
            for task in self.__tasks2stack:
                print(self.__tasks2stack[task])

    def getUserInput(self):
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
        return self.__spinPrio, self.__tasks2stack, self.__cpuInfo, self.__tasks2cores

#############################################################################################
# END OF FLEXSPIN TOOL
#############################################################################################

#############################################################################################
# START OF UPDATE SOURCE
#############################################################################################

    def updateSourceFiles(self):

        """Function used to update the source files of the cores"""

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

            indices = [-1, -1]

            if data:
                for line in data:
                    if line.find(SPIN_PRIO) != -1:
                        indices[0] = data.index(line)
                    if line.find(GLOBAL_TASK_ID) != -1:
                        indices[1] = data.index(line)
            else:
                sys.exit("No content present in file")

            if indices[0] != -1:
                spin_text = "const int "+SPIN_PRIO+"[] = {"
                for task in self.__tasks2cores:
                    if self.__tasks2cores[task] == cpu:
                        spin_text += "0x"+str(self.__spinPrio[cpu])+","
                spin_text = spin_text[:-1]
                spin_text += "};\n"
                data[indices[0]] = spin_text

            else:
                print("NOT found spin")

            if indices[1] != -1:
                glob_text = "const int " + GLOBAL_TASK_ID + "[] = {"
                for task in self.__tasks2cores:
                    if self.__tasks2cores[task] == cpu:
                        glob_text += str(task) + ","
                glob_text = glob_text[:-1]
                glob_text += "};\n"
                data[indices[1]] = glob_text

            else:
                print("NOT found glob")

            with open(fileName, 'w') as file:
                file.writelines(data)

    def updateOilFile(self):

        """Modify the stack configuration of the tasks in the OIL file"""

        with open(OIL_PATH, 'r') as file:
            data = file.readlines()


        task_pos = {}

        for task in self.__tasks2stack:
            task_info_start = False
            task_name = ""
            task_counter = -1
            brace_counter = 0
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
                    if self.__tasks2stack[task][1] == "SHARED":
                        output += "SHARED;\n"
                    elif self.__tasks2stack[task][1] == "PRIVATE":
                        output += "PRIVATE{ \tSYS_SIZE = 0x100; \t};\n"
                    data[i] = output
                    break

        with open(OIL_PATH, 'w') as file:
            file.writelines(data)


if __name__ == "__main__":

    flexObj = FlexTool(OIL_PATH)
    flexObj.parseOilFile()
    flexObj.initializeFlexSpinToolVars()
    flexObj.calculatePriorities()
    flexObj.getUserInput()
    flexObj.updateSourceFiles()
    flexObj.updateOilFile()

    del(flexObj)

    