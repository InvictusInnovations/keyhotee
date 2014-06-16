#pragma once
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/config.hpp>
#include <fc/uint128.hpp>
#include <fc/io/enum_type.hpp>
#include <stdint.h>

namespace bts { namespace blockchain {

  #define BTS_PRICE_PRECISION uint64_t(BTS_BLOCKCHAIN_MAX_SHARES*1000)
  struct price;

  /**
   *  An asset is a fixed point number with
   *  64.64 bit precision.  This is used
   *  for accumalating dividends and 
   *  calculating prices on the built-in exchange.
   */
  struct asset
  {
      asset():amount(0),asset_id(0){}
      explicit asset( share_type a, asset_id_type u = 0)
      :amount(a),asset_id(u){}

      asset& operator += ( const asset& o );
      asset& operator -= ( const asset& o );
      asset  operator *  ( const fc::uint128_t& fix6464 )const;
      asset  operator *  ( uint64_t constant )const
      {
         return asset( amount * constant );
      }
      asset  operator /  ( uint64_t constant )const
      {
         asset tmp(*this);
         tmp.amount /= constant;
         return tmp;
      }
      asset operator-()const { return asset( -amount, asset_id); }

      operator std::string()const;
       
      share_type     amount;
      asset_id_type  asset_id;
  };
  
  /**
   *  A price is the result of dividing 2 asset classes and has
   *  the fixed point format 64.64 and -1 equals infinite.
   */
  struct price
  {
      static const fc::uint128& one();
      static const fc::uint128& infinite();

      price() {}
      price( const fc::uint128_t& r, asset_id_type base, asset_id_type quote )
      :ratio(r),base_asset_id(base),quote_asset_id(quote){}

      price( const std::string& s );
      price( double a, asset_id_type base, asset_id_type quote );
      void set_ratio_from_string( const std::string& ratio_str );
      std::string ratio_string()const;
      operator std::string()const;
      operator double()const;

      fc::uint128_t ratio; // 64.64

      std::pair<asset_id_type,asset_id_type> asset_pair()const { return std::make_pair(base_asset_id,quote_asset_id); }

      asset_id_type base_asset_id;
      asset_id_type quote_asset_id;
  };

  inline bool operator == ( const asset& l, const asset& r ) { return l.amount == r.amount; }
  inline bool operator != ( const asset& l, const asset& r ) { return l.amount != r.amount; }
  inline bool operator <  ( const asset& l, const asset& r ) { return l.amount <  r.amount; }
  inline bool operator >  ( const asset& l, const asset& r ) { return l.amount >  r.amount; }
  inline bool operator <= ( const asset& l, const asset& r ) { return l.asset_id == r.asset_id && l.amount <= r.amount; }
  inline bool operator >= ( const asset& l, const asset& r ) { return l.asset_id == r.asset_id && l.amount >= r.amount; }
  inline asset operator +  ( const asset& l, const asset& r ) { return asset(l) += r; }
  inline asset operator -  ( const asset& l, const asset& r ) { return asset(l) -= r; }

  inline bool operator == ( const price& l, const price& r ) { return l.ratio == r.ratio; }
  inline bool operator != ( const price& l, const price& r ) { return l.ratio == r.ratio; }
  inline bool operator <  ( const price& l, const price& r ) { return l.ratio <  r.ratio; }
  inline bool operator >  ( const price& l, const price& r ) { return l.ratio >  r.ratio; }
  inline bool operator <= ( const price& l, const price& r ) { return l.ratio <= r.ratio && l.asset_pair() == r.asset_pair(); }
  inline bool operator >= ( const price& l, const price& r ) { return l.ratio >= r.ratio && l.asset_pair() == r.asset_pair(); }

  /**
   *  A price will reorder the asset types such that the
   *  asset type with the lower enum value is always the
   *  denominator.  Therefore  bts/usd and  usd/bts will
   *  always result in a price measured in usd/bts because
   *  bitasset_id_type::bit_shares <  bitasset_id_type::bit_usd.
   */
  price operator / ( const asset& a, const asset& b );

  /**
   *  Assuming a.type is either the numerator.type or denominator.type in
   *  the price equation, return the number of the other asset type that
   *  could be exchanged at price p.
   *
   *  ie:  p = 3 usd/bts & a = 4 bts then result = 12 usd
   *  ie:  p = 3 usd/bts & a = 4 usd then result = 1.333 bts 
   */
  asset operator * ( const asset& a, const price& p );



} } // bts::blockchain

namespace fc
{
 //  void to_variant( const bts::blockchain::asset& var,  variant& vo );
 //  void from_variant( const variant& var,  bts::blockchain::asset& vo );
   void to_variant( const bts::blockchain::price& var,  variant& vo );
   void from_variant( const variant& var,  bts::blockchain::price& vo );
}

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::blockchain::price, (ratio)(quote_asset_id)(base_asset_id) );
FC_REFLECT( bts::blockchain::asset, (amount)(asset_id) );

