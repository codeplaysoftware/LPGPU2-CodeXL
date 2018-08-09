import os
import platform


def get_computecpp_folder_and_version(full_path=False):
    # Get the current script path
    script_path = os.path.dirname(os.path.abspath(__file__))

    # List the folders starting with ComputeCpp
    computecpp_folder_list = list()
    for folder in os.listdir(script_path):
        if folder.startswith("ComputeCpp"):
            folder_fullpath = os.path.join(script_path, folder)
            if os.path.isdir(folder_fullpath):
                computecpp_folder_list.append(folder)

    if len(computecpp_folder_list) == 0:
        print("No ComputeCpp folder was found. \nMake sure there is at least one folder named using the format 'ComputeCpp-CE-<VERSION>'.")
        print("To download ComputeCpp, please go to https://developer.codeplay.com")
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

    computecpp_prefix = "ComputeCpp-CE"
    computecpp_version = computecpp_folder[len(computecpp_prefix) + 1:]

    if full_path:
        computecpp_folder = os.path.join(script_path, computecpp_folder)

    return computecpp_folder, computecpp_version


def setup_computecpp_linux():
    # Get the ComputeCpp folder and version
    computecpp_folder, computecpp_version = get_computecpp_folder_and_version()

    print("ComputeCpp Folder : {}".format(computecpp_folder))
    print("ComputeCpp Version: {}".format(computecpp_version))

def setup_computecpp_windows():
    # Get the ComputeCpp folder and Version
    computecpp_folder, computecpp_version = get_computecpp_folder_and_version()

    print("ComputeCpp Folder : {}".format(computecpp_folder))
    print("ComputeCpp Version: {}".format(computecpp_version))

    template_filename = "ComputeCpp-Global-Template.props"

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

    # Write "ComputeCpp-Global.props"
    props_filename = "ComputeCpp-Global.props"
    with open(props_filename, "w") as props_file:
        props_file.writelines(computecpp_props_content)

    print("'{}' generated.".format(props_filename))

def main():
    if platform.system() == "Windows":
        setup_computecpp_windows()
    elif platform.system() == "Linux":
        setup_computecpp_linux()
    else:
        print("OS '{}' is not supported. Exiting ...".format(platform.system))
        exit(1)


if __name__ == "__main__":
    main()
