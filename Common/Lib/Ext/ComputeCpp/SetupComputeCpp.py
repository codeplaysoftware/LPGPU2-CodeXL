import os
import platform
import shutil


# Get the current script path
SCRIPT_FOLDER_PATH = os.path.dirname(os.path.abspath(__file__))


def extract_computecpp_version(computecpp_folder):
    def __extract(__key, __line):
        if __key in __line:
            return int(__line.strip().split(" ")[-1])

    major_key = "COMPUTECPP_VERSION_MAJOR"
    minor_key = "COMPUTECPP_VERSION_MINOR"
    patch_key = "COMPUTECPP_VERSION_PATCH"
    is_ce_key = "COMPUTECPP_CE"

    major_version = None
    minor_version = None
    patch_version = None
    is_ce = False

    version_filepath = os.path.join(computecpp_folder, "include", "SYCL", "version.h")

    with open(version_filepath, "r") as version_file:
        version_file_contents = version_file.readlines()
        for line in version_file_contents:
            if major_version is None:
                major_version = __extract(major_key, line)
            if minor_version is None:
                minor_version = __extract(minor_key, line)
            if patch_version is None:
                patch_version = __extract(patch_key, line)
            if is_ce_key in line:
                is_ce = True

    return major_version, minor_version, patch_version, is_ce


def get_compute_cpp_folders():
    computecpp_folder_list = list()
    for folder in os.listdir(SCRIPT_FOLDER_PATH):
        if folder.startswith("ComputeCpp"):
            folder_fullpath = os.path.join(SCRIPT_FOLDER_PATH, folder)
            if os.path.isdir(folder_fullpath):
                computecpp_folder_list.append(folder)
    return computecpp_folder_list


def get_computecpp_folder_and_version(full_path=False):

    computecpp_folder_list = get_compute_cpp_folders()

    if len(computecpp_folder_list) == 0:
        print("ERROR: No ComputeCpp folder was found.")
        print("Make sure there is at least one folder named using the format 'ComputeCpp-<VERSION>'.")
        print("To download ComputeCpp, please go to https://developer.codeplay.com")
        if platform.system() == "Linux":
            print("After downloading the ComputeCpp package, extract the contents into '{}' and try to build again".format(SCRIPT_FOLDER_PATH))
        print("Exiting ...")
        exit(1)

    # If there is more than one, ask the user which one to use
    if len(computecpp_folder_list) > 1 and not full_path:
        print("Found more than one ComputeCpp folder. Which one to use?")

        for i in range(0, len(computecpp_folder_list)):
            print("[{index}]: {folder}".format(index=i, folder=computecpp_folder_list[i]))

        print()
        print("Which one to use?: ")
        chosen_folder_index = int(raw_input())

        if chosen_folder_index not in range(0, len(computecpp_folder_list)):
            print("The selected folder is not valid. Exiting ..")
            exit(1)

        computecpp_folder = computecpp_folder_list[chosen_folder_index]
    else:
        computecpp_folder = computecpp_folder_list[0]

    if full_path:
        computecpp_folder = os.path.join(SCRIPT_FOLDER_PATH, computecpp_folder)

    computecpp_version = computecpp_folder[len("ComputeCpp") + 1:]

    return computecpp_folder, computecpp_version


def setup_computecpp_windows():
    # List the folders starting with ComputeCpp
    computecpp_folder_list = get_compute_cpp_folders()

    if len(computecpp_folder_list) == 0:
        # If there no ComputeCpp folder in the current directoty, try to find one installed in the system
        computecpp_root_dir_varname = "COMPUTECPP_ROOT_DIR"
        if computecpp_root_dir_varname in os.environ:
            # ComputeCpp is installed in the system, lets copy the required files to the local folder
            # First find the ComputeCpp version that is installed
            computecpp_install_path = os.environ[computecpp_root_dir_varname]
            if os.path.exists(computecpp_install_path):
                major, minor, patch, is_ce = extract_computecpp_version(computecpp_install_path)

                # Create a folder that will contain ComputeCpp files
                if is_ce:
                    computecpp_foldername = "ComputeCpp-CE-{}.{}.{}-Win64".format(major, minor, patch)
                else:
                    computecpp_foldername = "ComputeCpp-{}.{}.{}-Win64".format(major, minor, patch)

                computecpp_folderpath = os.path.join(SCRIPT_FOLDER_PATH, computecpp_foldername)
                if not os.path.exists(computecpp_folderpath):
                    print("Copying ComputeCpp contents ...")
                    shutil.copytree(computecpp_install_path, computecpp_folderpath)

    # At this point, it will be guaranteed that we have at least one ComputeCpp installation in the current directory

    # Get the ComputeCpp folder and Version
    computecpp_folder, computecpp_version = get_computecpp_folder_and_version()

    print("ComputeCpp Folder : {}".format(computecpp_folder))
    print("ComputeCpp Version: {}".format(computecpp_version))

    template_filename = "ComputeCpp-Version-Template.props"

    if not os.path.exists(template_filename):
        print("Template '{}' not found. Exiting ...".format(template_filename))
        exit(1)

    computecpp_props_content = list()

    # Replace the version into ComputeCpp-Global-Template.props to generate the final version of the Props file
    with open(template_filename, "r") as template_file:
        template_contents = template_file.readlines()

        computecpp_version_tag = "COMPUTECPP_VERSION"

        # Replace COMPUTECPP_VERSION into the template file
        for line in template_contents:
            if computecpp_version_tag in line:
                line = line.replace(computecpp_version_tag, computecpp_version)
            computecpp_props_content.append(line)

    # Write "ComputeCpp-Version.props"
    props_filename = "ComputeCpp-Version.props"
    with open(props_filename, "w") as props_file:
        props_file.writelines(computecpp_props_content)

    print("'{}' generated.".format(props_filename))


def main():
    if platform.system() == "Windows":
        setup_computecpp_windows()
    elif platform.system() == "Linux":
        pass
    else:
        print("OS '{}' is not supported. Exiting ...".format(platform.system()))
        exit(1)


if __name__ == "__main__":
    main()
