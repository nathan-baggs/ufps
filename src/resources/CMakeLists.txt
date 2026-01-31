target_sources(ufpslib PUBLIC
	file_resource_loader.cpp
)

if(UFPS_USE_EMBEDDED_RESOURCE_LOADER)
  target_sources(ufpslib PUBLIC
    embedded_resource_loader.cpp
  )
endif()

