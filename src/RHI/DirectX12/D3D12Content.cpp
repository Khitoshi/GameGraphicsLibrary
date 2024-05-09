#include "D3D12Content.h"
#include "../Content/ContentToEngine.h"
#include "../Utilities/IOStream.h"
#include "D3D12Core.h"

namespace RHI::D3D12::CONTENT {


    namespace {

        struct position_view
        {
            D3D12_VERTEX_BUFFER_VIEW        position_buffer_view{};
            D3D12_INDEX_BUFFER_VIEW         index_buffer_view{};
        };

        struct element_view
        {
            D3D12_VERTEX_BUFFER_VIEW        element_buffer_view{};
            u32                             elements_type{};
            D3D_PRIMITIVE_TOPOLOGY          primitive_topology{};
        };

        Utility::free_list<ID3D12Resource*>     submesh_buffers{};
        Utility::free_list<position_view>       position_views{};
        Utility::free_list<element_view>        element_views{};
        std::mutex                              submesh_mutex{};

        D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(const ::CONTENT::PrimitveTopology::TYPE type)
        {
            //using namespace primal::content;
            assert(type < ::CONTENT::PrimitveTopology::COUNT);

            switch (type)
            {
            case ::CONTENT::PrimitveTopology::POINT_LIST:     return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case ::CONTENT::PrimitveTopology::LINE_LIST:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case ::CONTENT::PrimitveTopology::LINE_STRIP:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case ::CONTENT::PrimitveTopology::TRIANGLE_LIST:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case ::CONTENT::PrimitveTopology::TRIANGLE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            }

            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    } // anonymous namespace

    namespace SUBMESH
    {
        Id::id_type Add(const u8*& data)
        {
            Utility::BlobStreamReader blob{ (const u8*)data };

            const u32 element_size{ blob.Read<u32>() };
            const u32 vertex_count{ blob.Read<u32>() };
            const u32 elements_type{ blob.Read<u32>() };
            const u32 primitive_topology{ blob.Read<u32>() };
            const u32 index_count{ blob.Read<u32>() };
            const u32 index_size{ (vertex_count < (1 << 16)) ? sizeof(u16) : sizeof(u32) };

            // NOTE: element size may be 0, for position-only vertex formats.
            const u32 position_buffer_size{ sizeof(Math::v3) * vertex_count };
            const u32 element_buffer_size{ element_size * vertex_count };
            const u32 index_buffer_size{ index_size * vertex_count };

            constexpr u32 alignment{ D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT };
            const u32 aligned_position_buffer_size{ (u32)Math::AlignSizeUp<alignment>(position_buffer_size) };
            const u32 aligned_element_buffer_size{ (u32)Math::AlignSizeUp<alignment>(element_buffer_size) };
            const u32 total_buffer_size{ aligned_position_buffer_size + aligned_element_buffer_size + index_buffer_size };

            ID3D12Resource* resource{ D3DX::CreateBuffer(blob.Position(), total_buffer_size) };

            blob.Skip(total_buffer_size);
            data = blob.Position();

            position_view position_view{};
            position_view.position_buffer_view.BufferLocation = resource->GetGPUVirtualAddress();
            position_view.position_buffer_view.SizeInBytes = position_buffer_size;
            position_view.position_buffer_view.StrideInBytes = sizeof(Math::v3);

            position_view.index_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size + aligned_element_buffer_size;
            position_view.index_buffer_view.Format = (index_size == sizeof(u16)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
            position_view.index_buffer_view.SizeInBytes = position_buffer_size;

            element_view element_view{};
            if (element_size)
            {
                element_view.element_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size;
                element_view.element_buffer_view.SizeInBytes = element_buffer_size;
                element_view.element_buffer_view.StrideInBytes = element_size;
            }

            element_view.elements_type = elements_type;
            element_view.primitive_topology = GetD3DPrimitiveTopology((::CONTENT::PrimitveTopology::TYPE)primitive_topology);

            std::lock_guard lock{ submesh_mutex };
            submesh_buffers.add(resource);
            position_views.add(position_view);
            return element_views.add(element_view);
        }

        void Remove(Id::id_type id)
        {
            std::lock_guard lock{ submesh_mutex };
            position_views.remove(id);
            element_views.remove(id);

            Core::Detail::DeferredRelease(submesh_buffers[id]);
            submesh_buffers.remove(id);
        }
    }
}
