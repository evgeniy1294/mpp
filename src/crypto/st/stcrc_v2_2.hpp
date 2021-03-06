/**
  ***********************************************************
  @author Evgenii Fedoseev
  @file   src/crypto/st/stcrc_v2_2.hpp
  @brief  Compatible series: f3, g0, g4, f7, h7
  ***********************************************************
**/


#pragma once



//____________________INCLUDE_____________________//
#include <type_traits>
#include <limits>




namespace mpp::crc {
  inline namespace stcrc_v2_2 {
    
    
    class HardwareLogic final{
      private:
        
        template < class Model >
        constexpr static bool IsValidModel() noexcept(true) {
          static_assert( ((Model::kWidth == 7 )||
                          (Model::kWidth == 8 )||
                          (Model::kWidth == 16)||
                          (Model::kWidth == 32)), "Field 'Width' is incorrect, must be 7, 8, 16, 32");
         
          return true;
        }
        
        
      public:    
        
        template < class Model >
        static void Configure( CRC_TypeDef* crc ) noexcept(true) {
          static_assert( IsValidModel < Model >(), "This model not supported" );
          
          enum RevIn: std::uint32_t { 
            None     = 0b00 << CRC_CR_REV_IN_Pos,
            Byte     = 0b01 << CRC_CR_REV_IN_Pos,
            HalfWord = 0b10 << CRC_CR_REV_IN_Pos,
            Word     = 0b11 << CRC_CR_REV_IN_Pos
          };
          
          enum Width: std::uint32_t { 
            _32b = 0b00 << CRC_CR_POLYSIZE_Pos,
            _16b = 0b01 << CRC_CR_POLYSIZE_Pos,
            _8b  = 0b10 << CRC_CR_POLYSIZE_Pos,
            _7b  = 0b11 << CRC_CR_POLYSIZE_Pos
          };
              
          
          
          constexpr std::uint32_t CR_REV_OUT  = Model::kRefOut ? CRC_CR_REV_OUT : 0u ;
          constexpr std::uint32_t CR_REV_IN   = Model::kRefIn  ? RevIn::Byte : 0u ;       
          constexpr std::uint32_t CR_POLYSIZE = []() -> std::uint32_t {
            switch (Model::kWidth)
            {
              case 32u:
                return Width::_32b;
              case 16u:
                return Width::_16b;
              case 8u:
                return Width::_8b;
              default:
                return Width::_7b;
            }
          } ();
          
          
          constexpr std::uint32_t CR  = CR_REV_OUT | CR_REV_IN | CR_POLYSIZE;
           
          crc->CR   = CR;
          crc->IDR  = Model::kXorOut;
          crc->INIT = static_cast < std::uint32_t >(Model::kSeed);
          crc->POL  = static_cast < std::uint32_t >(Model::kPoly);
          crc->CR   = crc->CR | CRC_CR_RESET; 
        }
          
      
      
      
      template< typename T >
      static void Calculate( CRC_TypeDef* crc, const T* data, std::size_t size ) noexcept(true)
      {
        const std::uint8_t* end = reinterpret_cast< const std::uint8_t* >(data + static_cast<std::ptrdiff_t>(size));
        std::size_t unaligned = (reinterpret_cast<std::uintptr_t>(data) & 0b11u) ? 
                                  4u - reinterpret_cast<std::uintptr_t>(data) & 0b11u : 0u;
        
        const std::uint8_t*  pu8  = reinterpret_cast< const std::uint8_t*  >(data);
        while ( unaligned-- )
          *reinterpret_cast< volatile std::uint8_t* >(&crc->DR) = *pu8++;
          
        const std::uint32_t* pu32 = reinterpret_cast< const std::uint32_t* >(pu8);
        const std::uint32_t* pe32 = reinterpret_cast< const std::uint32_t* >( reinterpret_cast<std::uintptr_t>(end) & ~0b11 );
        while (pu32 < pe32)
           crc->DR = __REV(*pu32++);
          
        pu8 = reinterpret_cast< const std::uint8_t* >(pe32);
        while (pu8 < end)
          *reinterpret_cast< volatile std::uint8_t* >(&crc->DR) = *pu8++;
        
        return;
      }
        
        
        
        
     
        
      template< typename T >
      static void CalculateByByte( CRC_TypeDef* crc, const T* data, std::size_t size ) noexcept(true)
      { 
        const std::uint8_t* pu8 = reinterpret_cast< const std::uint8_t* >(data);
        const std::uint8_t* end = reinterpret_cast< const std::uint8_t* >(data + static_cast<std::ptrdiff_t>(size));

        while ( pu8 < end )
          *reinterpret_cast< volatile std::uint8_t* >(&crc->DR) = *pu8++;
        
        return;
      }
        
        
        
        
      static void CalculateFast( CRC_TypeDef* crc, const std::uint32_t* data, std::size_t size ) 
      noexcept(true)
      { 
        while ( size-- )
          crc->DR = *data++;
      }
        
        
        
      inline static std::uint32_t Finalize(CRC_TypeDef* crc) noexcept(true)
      {
        std::uint32_t ret = crc->DR;
        crc->CR = crc->CR | CRC_CR_RESET; 
        
        return ret ^ crc->IDR;
      }
      
    };
  }
} // namespace mpp::crypto
