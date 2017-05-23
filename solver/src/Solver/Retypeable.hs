-- Copyright (c) 2016 Leandro T. C. Melo (ltcmelo@gmail.com)
--
-- This library is free software; you can redistribute it and/or modify it under
-- the terms of the GNU Lesser General Public License as published by the Free
-- Software Foundation; either version 2.1 of the License, or (at your option)
-- any later version.
--
-- This library is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
-- for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this library; if not, write to the Free Software Foundation, Inc.,
-- 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA


-- Retyping capabilities.

module Solver.Retypeable where

import Data.BuiltIn
import Data.Maybe
import Data.Map (Map)
import qualified Data.Map as Map
import Data.Set (Set)
import qualified Data.Set as Set
import Data.Type
import Solver.SolverMonad
import Utils.Pretty


class Retypeable a where
  collect :: a -> [Ty]
  canonicalize :: TyIdx -> a -> a
  orphanize :: TyIdx -> a -> a

instance Retypeable a => Retypeable [a] where
  collect = concat . map (collect)
  canonicalize idx = map (canonicalize idx)
  orphanize idx = map (orphanize idx)

instance Retypeable Ty where
  collect (TyVar _) = []
  collect (TyCon _) = []
  collect (EnumTy t) = []
  collect (QualTy t) = collect t
  collect (Pointer t) = collect t
  collect (FunTy t tx) = collect t ++ collect tx
  collect t@(Struct fs _) = [t] ++ collect fs

  canonicalize _ t@(TyVar _) = t
  canonicalize _ t@(TyCon _) = t
  canonicalize _ t@(EnumTy _) = t
  canonicalize idx (QualTy t) = QualTy (canonicalize idx t)
  canonicalize idx (Pointer t) = Pointer (canonicalize idx t)
  canonicalize idx (FunTy t tx) = FunTy (canonicalize idx t) (canonicalize idx tx)
  canonicalize idx t@(Struct fs _) = maybe t (\k -> TyCon k) (Map.lookup t (ty2n idx))

  orphanize idx t@(TyVar v) = maybe orphan TyCon (Map.lookup t (ty2n idx))
  orphanize _ t@(TyCon _) = t
  orphanize _ t@(EnumTy _) = t
  orphanize idx (QualTy t) = QualTy (orphanize idx t)
  orphanize idx (Pointer t) = Pointer (orphanize idx t)
  orphanize idx (FunTy t tx) = FunTy (orphanize idx t) (orphanize idx tx)
  orphanize idx (Struct fs n) = Struct (orphanize idx fs) n

instance Retypeable Field where
  collect (Field _ t) = collect t
  canonicalize idx (Field n t) = Field n (canonicalize idx t)
  orphanize idx (Field n t) = Field n (orphanize idx t)


newtype TyIdx = TyIdx { ty2n :: Map Ty Name }

nullIdx :: TyIdx
nullIdx = TyIdx Map.empty

(%->) :: Ty -> Name -> TyIdx
t %-> n = TyIdx (Map.singleton t n)

(%%) :: TyIdx -> TyIdx -> TyIdx
nom1 %% nom2 = TyIdx $ (ty2n nom1) `Map.union` (ty2n nom2)

instance Pretty TyIdx where
  pprint = printer2 "%->" . ty2n


-- | Elminate top-level (and eventual recursive) alphas with an artificial name.
unalpha :: (Ty, Bool) -> SolverM (Ty, Bool)
unalpha p@(t@(Struct fs n), b)
  | isVar n = do
    n <- fakeName
    let n' = Name ("struct " ++ (unName n))
    return $ (Struct (check fs n') n', b)
  | otherwise = return p
 where
  check fs k = map (\(Field fn ft) -> Field fn (unalphaHelper k n ft)) fs
unalpha p = return p


-- | Elminate top-level (and eventual recursive) alphas preserving elaborated names.
unalpha2 :: Name -> Ty -> Ty
unalpha2 kn (Struct fs n) =
  Struct (check fs kn) kn
 where
  check fs k = map (\(Field fn ft) -> Field fn (unalphaHelper k n ft)) fs
unalpha2 _ t = t

-- Helper function for the unalpha versions above.
unalphaHelper k n t@(TyVar v)
  | n == v = TyCon k
  | otherwise = t
unalphaHelper _ _ t@(TyCon _) = t
unalphaHelper _ _ t@(EnumTy _) = t
unalphaHelper k n (QualTy t) = QualTy (unalphaHelper k n t)
unalphaHelper k n (Pointer t) = Pointer (unalphaHelper k n t)
unalphaHelper k n t@(FunTy _ _) = t
unalphaHelper _ _ (Struct _ _) = error "shouldn't have structs at this point"
