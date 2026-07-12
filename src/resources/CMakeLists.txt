target_sources(ufpslib PRIVATE
	file_resource_loader.cpp
)

if(UFPS_USE_EMBEDDED_RESOURCE_LOADER)
  target_sources(ufpslib PRIVATE
    embedded_resource_loader.cpp
  )
endif()

