#pragma once

#incl

std::optional<char *>
heph_get_hmodl_xattr(const std::string &path)
{
#ifdef HEPH_OSX
        char val[HMODL_XATTR_VALUE_SIZE];
        if (getxattr(path.c_str(), HMODL_XATTR_NAME, val, HMODL_XATTR_VALUE_SIZE, 0, 0) == -1)
        {
                HEPH_PRINT_ERROR("Failed to get xattr of: " << path);
                return {};
        }
        return val;
#endif

#ifdef HEPH_WINDOWS

#endif
}

HephResult
heph_set_hmodl_xattr(const std::string &path, char val[9])
{
#ifdef HEPH_OSX
        if (setxattr(path.c_str(), HMODL_XATTR_NAME, val, 9, 0, 0) == -1)
        {
                HEPH_PRINT_ERROR("Cannot set hmodl xattr.")
                return HephResult::Failure;
        }
        return HephResult::Success;
#endif

#ifdef HEPH_WINDOWS

#endif
}