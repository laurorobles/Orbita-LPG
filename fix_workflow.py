import re

with open(".github/workflows/build.yml", "r") as f:
    yml = f.read()

# Replace ashley-taylor with seanmiddleditch
yml = yml.replace("ashley-taylor/setup-ninja@v1.0.0", "seanmiddleditch/gha-setup-ninja@master")

with open(".github/workflows/build.yml", "w") as f:
    f.write(yml)
