class Component(object):
    """
        Base component that need to be used by all components to be used.
    """
    def setup(self):
        """
            Perform setup.
        :return: msg if the Setup Failed else None.
        """
        return None

    def perform(self):
        """
            Perform the tasks needed for the component.
        :return:
        """
        raise NotImplementedError("Perform needs to be implemented.")

    def cleanup(self):
        """
            Perform clean up.
        :return: msg if the cleanup Failed else None.
        """
        return None

    def get_name(self):
        """
            Get the name of the component
        :return: String representing component name.
        """
        return "NoName"

    def is_critical(self):
        """
            Is the component critical?
            If true, then failure of this component is treated as fatal.
        :return: True if the component is critical else false.
        """
        return False


class Bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def log_info(*args):
    log_str = Bcolors.OKBLUE + "[*] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    log_str += Bcolors.ENDC
    print log_str


def log_error(*args):
    log_str = Bcolors.FAIL + "[!] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    log_str += Bcolors.ENDC
    print log_str


def log_warning(*args):
    log_str = Bcolors.WARNING + "[?] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    log_str += Bcolors.ENDC
    print log_str


def log_success(*args):
    log_str = Bcolors.OKGREEN + "[+] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    log_str += Bcolors.ENDC
    print log_str
