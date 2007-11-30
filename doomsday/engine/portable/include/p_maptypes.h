/ *   G e n e r a t e d   b y   . . / . . / e n g i n e / s c r i p t s / m a k e d m t . p y   * / 
 
 # i f n d e f   _ _ D O O M S D A Y _ P L A Y _ M A P _ D A T A _ T Y P E S _ H _ _ 
 # d e f i n e   _ _ D O O M S D A Y _ P L A Y _ M A P _ D A T A _ T Y P E S _ H _ _ 
 
 # i n c l u d e   " p _ m a p d a t a . h " 
 
 # d e f i n e   L O _ p r e v           l i n k [ 0 ] 
 # d e f i n e   L O _ n e x t           l i n k [ 1 ] 
 
 t y p e d e f   s t r u c t   l i n e o w n e r _ s   { 
         s t r u c t   l i n e _ s   * l i n e ; 
         s t r u c t   l i n e o w n e r _ s   * l i n k [ 2 ] ;         / /   { p r e v ,   n e x t }   ( i . e .   { a n t i c l k ,   c l k } ) . 
         b i n a n g l e _ t             a n g l e ;                     / /   b e t w e e n   t h i s   a n d   n e x t   c l o c k w i s e . 
 }   l i n e o w n e r _ t ; 
 
 # d e f i n e   V _ p o s                                       v . p o s 
 
 t y p e d e f   s t r u c t   v e r t e x _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         f v e r t e x _ t                       v ; 
         u n s i g n e d   i n t                 n u m l i n e o w n e r s ;   / /   N u m b e r   o f   l i n e   o w n e r s . 
         l i n e o w n e r _ t *                 l i n e o w n e r s ;         / /   L i n e o w n e r   b a s e   p t r   [ n u m l i n e o w n e r s ]   s i z e .   A   d o u b l y ,   c i r c u l a r l y   l i n k e d   l i s t .   T h e   b a s e   i s   t h e   l i n e   w i t h   t h e   l o w e s t   a n g l e   a n d   t h e   n e x t - m o s t   w i t h   t h e   l a r g e s t   a n g l e . 
         b o o l e a n                           a n c h o r e d ;             / /   O n e   o r   m o r e   o f   o u r   l i n e   o w n e r s   a r e   o n e - s i d e d . 
 }   v e r t e x _ t ; 
 
 / /   H e l p e r   m a c r o s   f o r   a c c e s s i n g   s e g   d a t a   e l e m e n t s . 
 # d e f i n e   F R O N T   0 
 # d e f i n e   B A C K     1 
 
 # d e f i n e   S G _ v ( n )                                   v [ ( n ) ] 
 # d e f i n e   S G _ v p o s ( n )                             S G _ v ( n ) - > V _ p o s 
 
 # d e f i n e   S G _ v 1                                       S G _ v ( 0 ) 
 # d e f i n e   S G _ v 1 p o s                                 S G _ v ( 0 ) - > V _ p o s 
 
 # d e f i n e   S G _ v 2                                       S G _ v ( 1 ) 
 # d e f i n e   S G _ v 2 p o s                                 S G _ v ( 1 ) - > V _ p o s 
 
 # d e f i n e   S G _ s e c t o r ( n )                         s e c [ ( n ) ] 
 # d e f i n e   S G _ f r o n t s e c t o r                     S G _ s e c t o r ( F R O N T ) 
 # d e f i n e   S G _ b a c k s e c t o r                       S G _ s e c t o r ( B A C K ) 
 
 / /   S e g   f l a g s 
 # d e f i n e   S E G F _ P O L Y O B J                         0 x 1   / /   S e g   i s   p a r t   o f   a   p o l y   o b j e c t . 
 
 / /   S e g   f r a m e   f l a g s 
 # d e f i n e   S E G I N F _ F A C I N G F R O N T             0 x 0 0 0 1 
 # d e f i n e   S E G I N F _ B A C K S E C S K Y F I X         0 x 0 0 0 2 
 
 t y p e d e f   s t r u c t   s e g _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         s t r u c t   v e r t e x _ s *         v [ 2 ] ;                     / /   [ S t a r t ,   E n d ]   o f   t h e   s e g m e n t . 
         f l o a t                               l e n g t h ;                 / /   A c c u r a t e   l e n g t h   o f   t h e   s e g m e n t   ( v 1   - >   v 2 ) . 
         f l o a t                               o f f s e t ; 
         s t r u c t   s i d e _ s *             s i d e d e f ; 
         s t r u c t   l i n e _ s *             l i n e d e f ; 
         s t r u c t   s e c t o r _ s *         s e c [ 2 ] ; 
         s t r u c t   s u b s e c t o r _ s *   s u b s e c t o r ; 
         s t r u c t   s e g _ s *               b a c k s e g ; 
         a n g l e _ t                           a n g l e ; 
         b y t e                                 s i d e ;                     / /   0 = f r o n t ,   1 = b a c k 
         b y t e                                 f l a g s ; 
         s h o r t                               f r a m e f l a g s ; 
         s t r u c t   b i a s t r a c k e r _ s   t r a c k e r [ 3 ] ;       / /   0 = m i d d l e ,   1 = t o p ,   2 = b o t t o m 
         s t r u c t   v e r t e x i l l u m _ s   i l l u m [ 3 ] [ 4 ] ; 
         u n s i g n e d   i n t                 u p d a t e d ; 
         s t r u c t   b i a s a f f e c t i o n _ s   a f f e c t e d [ M A X _ B I A S _ A F F E C T E D ] ; 
 }   s e g _ t ; 
 
 # d e f i n e   S U B F _ M I D P O I N T                   0 x 8 0         / /   M i d p o i n t   i s   t r i - f a n   c e n t r e . 
 
 t y p e d e f   s t r u c t   s u b s e c t o r _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         s t r u c t   s e c t o r _ s *         s e c t o r ; 
         u n s i g n e d   i n t                 s e g c o u n t ; 
         s t r u c t   s e g _ s * *             s e g s ;                     / /   [ s e g c o u n t ]   s i z e . 
         s t r u c t   p o l y o b j _ s *       p o l y ;                     / /   N U L L ,   i f   t h e r e   i s   n o   p o l y o b j . 
         i n t                                   f l a g s ; 
         f v e r t e x _ t                       b b o x [ 2 ] ;               / /   M i n   a n d   m a x   p o i n t s . 
         f v e r t e x _ t                       m i d p o i n t ;             / /   C e n t e r   o f   v e r t i c e s . 
         s t r u c t   s u b p l a n e i n f o _ s * *   p l a n e s ; 
         u n s i g n e d   s h o r t             n u m v e r t i c e s ; 
         s t r u c t   f v e r t e x _ s * *     v e r t i c e s ;             / /   [ n u m v e r t i c e s ]   s i z e 
         i n t                                   v a l i d C o u n t ; 
         s t r u c t   s h a d o w l i n k _ s *   s h a d o w s ; 
         u n s i g n e d   i n t                 g r o u p ; 
         u n s i g n e d   i n t                 r e v e r b [ N U M _ R E V E R B _ D A T A ] ; 
 }   s u b s e c t o r _ t ; 
 
 / /   S u r f a c e   f l a g s . 
 # d e f i n e   S U F _ T E X F I X             0 x 1                   / /   C u r r e n t   t e x t u r e   i s   a   f i x   r e p l a c e m e n t 
                                                                         / /   ( n o t   s e n t   t o   c l i e n t s ,   r e t u r n e d   v i a   D M U   e t c ) . 
 # d e f i n e   S U F _ G L O W                 0 x 2                   / /   S u r f a c e   g l o w s   ( f u l l   b r i g h t ) . 
 # d e f i n e   S U F _ B L E N D               0 x 4                   / /   S u r f a c e   p o s s i b l y   h a s   a   b l e n d e d   t e x t u r e . 
 # d e f i n e   S U F _ N O _ R A D I O         0 x 8                   / /   N o   f a k e r a d i o   f o r   t h i s   s u r f a c e . 
 
 / /   S u r f a c e   f r a m e   f l a g s 
 # d e f i n e   S U F I N F _ P V I S           0 x 0 0 0 1 
 
 t y p e d e f   s t r u c t   s u r f a c e _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         i n t                                   f l a g s ;                   / /   S U F _   f l a g s 
         i n t                                   o l d f l a g s ; 
         s t r u c t   m a t e r i a l _ s       * m a t e r i a l ; 
         s t r u c t   m a t e r i a l _ s       * o l d m a t e r i a l ; 
         b l e n d m o d e _ t                   b l e n d m o d e ; 
         f l o a t                               n o r m a l [ 3 ] ;           / /   S u r f a c e   n o r m a l 
         f l o a t                               o l d n o r m a l [ 3 ] ; 
         f l o a t                               o f f s e t [ 2 ] ;           / /   [ X ,   Y ]   P l a n a r   o f f s e t   t o   s u r f a c e   m a t e r i a l   o r i g i n . 
         f l o a t                               o l d o f f s e t [ 2 ] ; 
         f l o a t                               r g b a [ 4 ] ;               / /   S u r f a c e   c o l o r   t i n t 
         f l o a t                               o l d r g b a [ 4 ] ; 
         s h o r t                               f r a m e f l a g s ; 
 }   s u r f a c e _ t ; 
 
 t y p e d e f   e n u m   { 
         P L N _ F L O O R , 
         P L N _ C E I L I N G , 
         N U M _ P L A N E _ T Y P E S 
 }   p l a n e t y p e _ t ; 
 
 t y p e d e f   s t r u c t   s k y f i x _ s   { 
         f l o a t   o f f s e t ; 
 }   s k y f i x _ t ; 
 
 # d e f i n e   P S _ n o r m a l                               s u r f a c e . n o r m a l 
 # d e f i n e   P S _ m a t e r i a l                           s u r f a c e . m a t e r i a l 
 # d e f i n e   P S _ o f f s e t                               s u r f a c e . o f f s e t 
 
 t y p e d e f   s t r u c t   p l a n e _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         f l o a t                               h e i g h t ;                 / /   C u r r e n t   h e i g h t 
         f l o a t                               o l d h e i g h t [ 2 ] ; 
         s u r f a c e _ t                       s u r f a c e ; 
         f l o a t                               g l o w ;                     / /   G l o w   a m o u n t 
         f l o a t                               g l o w r g b [ 3 ] ;         / /   G l o w   c o l o r 
         f l o a t                               t a r g e t ;                 / /   T a r g e t   h e i g h t 
         f l o a t                               s p e e d ;                   / /   M o v e   s p e e d 
         d e g e n m o b j _ t                   s o u n d o r g ;             / /   S o u n d   o r i g i n   f o r   p l a n e 
         s t r u c t   s e c t o r _ s *         s e c t o r ;                 / /   O w n e r   o f   t h e   p l a n e   ( t e m p ) 
         f l o a t                               v i s h e i g h t ;           / /   V i s i b l e   p l a n e   h e i g h t   ( s m o o t h e d ) 
         f l o a t                               v i s o f f s e t ; 
 }   p l a n e _ t ; 
 
 / /   H e l p e r   m a c r o s   f o r   a c c e s s i n g   s e c t o r   f l o o r / c e i l i n g   p l a n e   d a t a   e l e m e n t s . 
 # d e f i n e   S P _ p l a n e ( n )                           p l a n e s [ ( n ) ] 
 
 # d e f i n e   S P _ p l a n e s u r f a c e ( n )             S P _ p l a n e ( n ) - > s u r f a c e 
 # d e f i n e   S P _ p l a n e h e i g h t ( n )               S P _ p l a n e ( n ) - > h e i g h t 
 # d e f i n e   S P _ p l a n e n o r m a l ( n )               S P _ p l a n e ( n ) - > s u r f a c e . n o r m a l 
 # d e f i n e   S P _ p l a n e m a t e r i a l ( n )           S P _ p l a n e ( n ) - > s u r f a c e . m a t e r i a l 
 # d e f i n e   S P _ p l a n e o f f s e t ( n )               S P _ p l a n e ( n ) - > s u r f a c e . o f f s e t 
 # d e f i n e   S P _ p l a n e r g b ( n )                     S P _ p l a n e ( n ) - > s u r f a c e . r g b a 
 # d e f i n e   S P _ p l a n e g l o w ( n )                   S P _ p l a n e ( n ) - > g l o w 
 # d e f i n e   S P _ p l a n e g l o w r g b ( n )             S P _ p l a n e ( n ) - > g l o w r g b 
 # d e f i n e   S P _ p l a n e t a r g e t ( n )               S P _ p l a n e ( n ) - > t a r g e t 
 # d e f i n e   S P _ p l a n e s p e e d ( n )                 S P _ p l a n e ( n ) - > s p e e d 
 # d e f i n e   S P _ p l a n e s o u n d o r g ( n )           S P _ p l a n e ( n ) - > s o u n d o r g 
 # d e f i n e   S P _ p l a n e v i s h e i g h t ( n )         S P _ p l a n e ( n ) - > v i s h e i g h t 
 
 # d e f i n e   S P _ c e i l s u r f a c e                     S P _ p l a n e s u r f a c e ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l h e i g h t                       S P _ p l a n e h e i g h t ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l n o r m a l                       S P _ p l a n e n o r m a l ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l m a t e r i a l                   S P _ p l a n e m a t e r i a l ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l o f f s e t                       S P _ p l a n e o f f s e t ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l r g b                             S P _ p l a n e r g b ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l g l o w                           S P _ p l a n e g l o w ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l g l o w r g b                     S P _ p l a n e g l o w r g b ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l t a r g e t                       S P _ p l a n e t a r g e t ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l s p e e d                         S P _ p l a n e s p e e d ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l s o u n d o r g                   S P _ p l a n e s o u n d o r g ( P L N _ C E I L I N G ) 
 # d e f i n e   S P _ c e i l v i s h e i g h t                 S P _ p l a n e v i s h e i g h t ( P L N _ C E I L I N G ) 
 
 # d e f i n e   S P _ f l o o r s u r f a c e                   S P _ p l a n e s u r f a c e ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r h e i g h t                     S P _ p l a n e h e i g h t ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r n o r m a l                     S P _ p l a n e n o r m a l ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r m a t e r i a l                 S P _ p l a n e m a t e r i a l ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r o f f s e t                     S P _ p l a n e o f f s e t ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r r g b                           S P _ p l a n e r g b ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r g l o w                         S P _ p l a n e g l o w ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r g l o w r g b                   S P _ p l a n e g l o w r g b ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r t a r g e t                     S P _ p l a n e t a r g e t ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r s p e e d                       S P _ p l a n e s p e e d ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r s o u n d o r g                 S P _ p l a n e s o u n d o r g ( P L N _ F L O O R ) 
 # d e f i n e   S P _ f l o o r v i s h e i g h t               S P _ p l a n e v i s h e i g h t ( P L N _ F L O O R ) 
 
 # d e f i n e   S _ s k y f i x ( n )                           s k y f i x [ ( n ) ] 
 # d e f i n e   S _ f l o o r s k y f i x                       S _ s k y f i x ( P L N _ F L O O R ) 
 # d e f i n e   S _ c e i l s k y f i x                         S _ s k y f i x ( P L N _ C E I L I N G ) 
 
 / /   S e c t o r   f r a m e   f l a g s 
 # d e f i n e   S I F _ V I S I B L E                   0 x 1           / /   S e c t o r   i s   v i s i b l e   o n   t h i s   f r a m e . 
 # d e f i n e   S I F _ F R A M E _ C L E A R           0 x 1           / /   F l a g s   t o   c l e a r   b e f o r e   e a c h   f r a m e . 
 # d e f i n e   S I F _ L I G H T _ C H A N G E D       0 x 2 
 
 / /   S e c t o r   f l a g s . 
 # d e f i n e   S E C F _ I N V I S _ F L O O R         0 x 1 
 # d e f i n e   S E C F _ I N V I S _ C E I L I N G     0 x 2 
 
 t y p e d e f   s t r u c t   s s e c g r o u p _ s   { 
         s t r u c t   s e c t o r _ s * *       l i n k e d ;           / /   [ s e c t o r - > p l a n e c o u n t + 1 ]   s i z e . 
                                                                         / /   P l a n e   a t t a c h e d   t o   a n o t h e r   s e c t o r . 
 }   s s e c g r o u p _ t ; 
 
 t y p e d e f   s t r u c t   s e c t o r _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         f l o a t                               l i g h t l e v e l ; 
         f l o a t                               o l d l i g h t l e v e l ; 
         f l o a t                               r g b [ 3 ] ; 
         f l o a t                               o l d r g b [ 3 ] ; 
         i n t                                   v a l i d C o u n t ;         / /   i f   = =   v a l i d C o u n t ,   a l r e a d y   c h e c k e d . 
         s t r u c t   m o b j _ s *             m o b j L i s t ;             / /   L i s t   o f   m o b j s   i n   t h e   s e c t o r . 
         u n s i g n e d   i n t                 l i n e c o u n t ; 
         s t r u c t   l i n e _ s * *           L i n e s ;                   / /   [ l i n e c o u n t + 1 ]   s i z e . 
         s t r u c t   s u b s e c t o r _ s * *   s u b s e c t o r s ;       / /   [ s u b s c o u n t + 1 ]   s i z e . 
         u n s i g n e d   i n t                 n u m R e v e r b S S e c A t t r i b u t o r s ; 
         s t r u c t   s u b s e c t o r _ s * *   r e v e r b S S e c s ;     / /   [ n u m R e v e r b S S e c A t t r i b u t o r s ]   s i z e . 
         u n s i g n e d   i n t                 s u b s g r o u p c o u n t ; 
         s s e c g r o u p _ t *                 s u b s g r o u p s ;         / /   [ s u b s g r o u p c o u n t + 1 ]   s i z e . 
         s k y f i x _ t                         s k y f i x [ 2 ] ;           / /   f l o o r ,   c e i l i n g . 
         d e g e n m o b j _ t                   s o u n d o r g ; 
         f l o a t                               r e v e r b [ N U M _ R E V E R B _ D A T A ] ; 
         u n s i g n e d   i n t                 b l o c k b o x [ 4 ] ;       / /   M a p b l o c k   b o u n d i n g   b o x . 
         u n s i g n e d   i n t                 p l a n e c o u n t ; 
         s t r u c t   p l a n e _ s * *         p l a n e s ;                 / /   [ p l a n e c o u n t + 1 ]   s i z e . 
         s t r u c t   s e c t o r _ s *         c o n t a i n s e c t o r ;   / /   S e c t o r   t h a t   c o n t a i n s   t h i s   ( i f   a n y ) . 
         b o o l e a n                           p e r m a n e n t l i n k ; 
         b o o l e a n                           u n c l o s e d ;             / /   A n   u n c l o s e d   s e c t o r   ( s o m e   s o r t   o f   f a n c y   h a c k ) . 
         b o o l e a n                           s e l f R e f H a c k ;       / /   A   s e l f - r e f e r e n c i n g   h a c k   s e c t o r   w h i c h   I S N T   e n c l o s e d   b y   t h e   s e c t o r   r e f e r e n c e d .   B o u n d i n g   b o x   f o r   t h e   s e c t o r . 
         f l o a t                               b b o x [ 4 ] ;               / /   B o u n d i n g   b o x   f o r   t h e   s e c t o r 
         i n t                                   f r a m e f l a g s ; 
         i n t                                   a d d s p r i t e c o u n t ;   / /   f r a m e   n u m b e r   o f   l a s t   R _ A d d S p r i t e s 
         s t r u c t   s e c t o r _ s *         l i g h t s o u r c e ;       / /   M a i n   s k y   l i g h t   s o u r c e 
         u n s i g n e d   i n t                 b l o c k c o u n t ;         / /   N u m b e r   o f   g r i d b l o c k s   i n   t h e   s e c t o r . 
         u n s i g n e d   i n t                 c h a n g e d b l o c k c o u n t ;   / /   N u m b e r   o f   b l o c k s   t o   m a r k   c h a n g e d . 
         u n s i g n e d   s h o r t *           b l o c k s ;                 / /   L i g h t   g r i d   b l o c k   i n d i c e s . 
 }   s e c t o r _ t ; 
 
 / /   P a r t s   o f   a   w a l l   s e g m e n t . 
 t y p e d e f   e n u m   s e g s e c t i o n _ e   { 
         S E G _ M I D D L E , 
         S E G _ T O P , 
         S E G _ B O T T O M 
 }   s e g s e c t i o n _ t ; 
 
 / /   H e l p e r   m a c r o s   f o r   a c c e s s i n g   s i d e d e f   t o p / m i d d l e / b o t t o m   s e c t i o n   d a t a   e l e m e n t s . 
 # d e f i n e   S W _ s u r f a c e ( n )                       s e c t i o n s [ ( n ) ] 
 # d e f i n e   S W _ s u r f a c e f l a g s ( n )             S W _ s u r f a c e ( n ) . f l a g s 
 # d e f i n e   S W _ s u r f a c e m a t e r i a l ( n )       S W _ s u r f a c e ( n ) . m a t e r i a l 
 # d e f i n e   S W _ s u r f a c e n o r m a l ( n )           S W _ s u r f a c e ( n ) . n o r m a l 
 # d e f i n e   S W _ s u r f a c e o f f s e t ( n )           S W _ s u r f a c e ( n ) . o f f s e t 
 # d e f i n e   S W _ s u r f a c e r g b a ( n )               S W _ s u r f a c e ( n ) . r g b a 
 # d e f i n e   S W _ s u r f a c e b l e n d m o d e ( n )     S W _ s u r f a c e ( n ) . b l e n d m o d e 
 
 # d e f i n e   S W _ m i d d l e s u r f a c e                 S W _ s u r f a c e ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e f l a g s                     S W _ s u r f a c e f l a g s ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e m a t e r i a l               S W _ s u r f a c e m a t e r i a l ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e n o r m a l                   S W _ s u r f a c e n o r m a l ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e t e x m o v e                 S W _ s u r f a c e t e x m o v e ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e o f f s e t                   S W _ s u r f a c e o f f s e t ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e r g b a                       S W _ s u r f a c e r g b a ( S E G _ M I D D L E ) 
 # d e f i n e   S W _ m i d d l e b l e n d m o d e             S W _ s u r f a c e b l e n d m o d e ( S E G _ M I D D L E ) 
 
 # d e f i n e   S W _ t o p s u r f a c e                       S W _ s u r f a c e ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p f l a g s                           S W _ s u r f a c e f l a g s ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p m a t e r i a l                     S W _ s u r f a c e m a t e r i a l ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p n o r m a l                         S W _ s u r f a c e n o r m a l ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p t e x m o v e                       S W _ s u r f a c e t e x m o v e ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p o f f s e t                         S W _ s u r f a c e o f f s e t ( S E G _ T O P ) 
 # d e f i n e   S W _ t o p r g b a                             S W _ s u r f a c e r g b a ( S E G _ T O P ) 
 
 # d e f i n e   S W _ b o t t o m s u r f a c e                 S W _ s u r f a c e ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m f l a g s                     S W _ s u r f a c e f l a g s ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m m a t e r i a l               S W _ s u r f a c e m a t e r i a l ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m n o r m a l                   S W _ s u r f a c e n o r m a l ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m t e x m o v e                 S W _ s u r f a c e t e x m o v e ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m o f f s e t                   S W _ s u r f a c e o f f s e t ( S E G _ B O T T O M ) 
 # d e f i n e   S W _ b o t t o m r g b a                       S W _ s u r f a c e r g b a ( S E G _ B O T T O M ) 
 
 / /   S i d e d e f   f l a g s 
 # d e f i n e   S D F _ B L E N D T O P T O M I D               0 x 0 1 
 # d e f i n e   S D F _ B L E N D M I D T O T O P               0 x 0 2 
 # d e f i n e   S D F _ B L E N D M I D T O B O T T O M         0 x 0 4 
 # d e f i n e   S D F _ B L E N D B O T T O M T O M I D         0 x 0 8 
 
 t y p e d e f   s t r u c t   s i d e _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         s u r f a c e _ t                       s e c t i o n s [ 3 ] ; 
         u n s i g n e d   i n t                 s e g c o u n t ; 
         s t r u c t   s e g _ s * *             s e g s ;                     / /   [ s e g c o u n t ]   s i z e ,   s e g s   a r r a n g e d   l e f t > r i g h t 
         s t r u c t   s e c t o r _ s *         s e c t o r ; 
         s h o r t                               f l a g s ; 
         i n t                                   f a k e R a d i o U p d a t e C o u n t ;   / /   f r a m e   n u m b e r   o f   l a s t   u p d a t e 
         s h a d o w c o r n e r _ t             t o p C o r n e r s [ 2 ] ; 
         s h a d o w c o r n e r _ t             b o t t o m C o r n e r s [ 2 ] ; 
         s h a d o w c o r n e r _ t             s i d e C o r n e r s [ 2 ] ; 
         e d g e s p a n _ t                     s p a n s [ 2 ] ;             / /   [ l e f t ,   r i g h t ] 
 }   s i d e _ t ; 
 
 / /   H e l p e r   m a c r o s   f o r   a c c e s s i n g   l i n e d e f   d a t a   e l e m e n t s . 
 # d e f i n e   L _ v ( n )                                     v [ ( n ) ] 
 # d e f i n e   L _ v p o s ( n )                               v [ ( n ) ] - > V _ p o s 
 
 # d e f i n e   L _ v 1                                         L _ v ( 0 ) 
 # d e f i n e   L _ v 1 p o s                                   L _ v ( 0 ) - > V _ p o s 
 
 # d e f i n e   L _ v 2                                         L _ v ( 1 ) 
 # d e f i n e   L _ v 2 p o s                                   L _ v ( 1 ) - > V _ p o s 
 
 # d e f i n e   L _ v o ( n )                                   v o [ ( n ) ] 
 # d e f i n e   L _ v o 1                                       L _ v o ( 0 ) 
 # d e f i n e   L _ v o 2                                       L _ v o ( 1 ) 
 
 # d e f i n e   L _ s i d e ( n )                               s i d e s [ ( n ) ] 
 # d e f i n e   L _ f r o n t s i d e                           L _ s i d e ( F R O N T ) 
 # d e f i n e   L _ b a c k s i d e                             L _ s i d e ( B A C K ) 
 # d e f i n e   L _ s e c t o r ( n )                           s i d e s [ ( n ) ] - > s e c t o r 
 # d e f i n e   L _ f r o n t s e c t o r                       L _ s e c t o r ( F R O N T ) 
 # d e f i n e   L _ b a c k s e c t o r                         L _ s e c t o r ( B A C K ) 
 
 / /   L i n e   f l a g s 
 # d e f i n e   L I N E F _ S E L F R E F                       0 x 1   / /   F r o n t   a n d   b a c k   s e c t o r s   o f   t h i s   l i n e   a r e   t h e   s a m e . 
 
 t y p e d e f   s t r u c t   l i n e _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         s t r u c t   v e r t e x _ s *         v [ 2 ] ; 
         i n t                                   f l a g s ; 
         s h o r t                               m a p f l a g s ;             / /   M F _ *   f l a g s ,   r e a d   f r o m   t h e   L I N E D E F S ,   m a p   d a t a   l u m p . 
         f l o a t                               d x ; 
         f l o a t                               d y ; 
         s l o p e t y p e _ t                   s l o p e t y p e ; 
         i n t                                   v a l i d C o u n t ; 
         s t r u c t   s i d e _ s *             s i d e s [ 2 ] ; 
         f l o a t                               b b o x [ 4 ] ; 
         s t r u c t   l i n e o w n e r _ s *   v o [ 2 ] ;                   / /   L i n k s   t o   v e r t e x   l i n e   o w n e r   n o d e s   [ l e f t ,   r i g h t ] 
         f l o a t                               l e n g t h ;                 / /   A c c u r a t e   l e n g t h 
         b i n a n g l e _ t                     a n g l e ;                   / /   C a l c u l a t e d   f r o m   f r o n t   s i d e ' s   n o r m a l 
         b o o l e a n                           m a p p e d [ D D M A X P L A Y E R S ] ;   / /   W h e t h e r   t h e   l i n e   h a s   b e e n   m a p p e d   b y   e a c h   p l a y e r   y e t . 
 }   l i n e _ t ; 
 
 t y p e d e f   s t r u c t   p o l y o b j _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         u n s i g n e d   i n t                 i d x ;                       / /   I d x   o f   p o l y o b j e c t 
         u n s i g n e d   i n t                 n u m s e g s ; 
         s t r u c t   s e g _ s * *             s e g s ; 
         i n t                                   v a l i d C o u n t ; 
         d e g e n m o b j _ t                   s t a r t S p o t ; 
         a n g l e _ t                           a n g l e ; 
         i n t                                   t a g ;                       / /   R e f e r e n c e   t a g   a s s i g n e d   i n   H e r e t i c E d 
         f v e r t e x _ t *                     o r i g i n a l P t s ;       / /   U s e d   a s   t h e   b a s e   f o r   t h e   r o t a t i o n s 
         f v e r t e x _ t *                     p r e v P t s ;               / /   U s e   t o   r e s t o r e   t h e   o l d   p o i n t   v a l u e s 
         v e c 2 _ t                             b o x [ 2 ] ; 
         f v e r t e x _ t                       d e s t ;                     / /   D e s t i n a t i o n   X Y 
         f l o a t                               s p e e d ;                   / /   M o v e m e n t   s p e e d . 
         a n g l e _ t                           d e s t A n g l e ;           / /   D e s t i n a t i o n   a n g l e . 
         a n g l e _ t                           a n g l e S p e e d ;         / /   R o t a t i o n   s p e e d . 
         b o o l e a n                           c r u s h ;                   / /   S h o u l d   t h e   p o l y o b j   a t t e m p t   t o   c r u s h   m o b j s ? 
         i n t                                   s e q T y p e ; 
         v o i d *                               s p e c i a l d a t a ;       / /   p o i n t e r   a   t h i n k e r ,   i f   t h e   p o l y   i s   m o v i n g 
 }   p o l y o b j _ t ; 
 
 t y p e d e f   s t r u c t   n o d e _ s   { 
         r u n t i m e _ m a p d a t a _ h e a d e r _ t   h e a d e r ; 
         f l o a t                               x ;                           / /   P a r t i t i o n   l i n e . 
         f l o a t                               y ;                           / /   P a r t i t i o n   l i n e . 
         f l o a t                               d x ;                         / /   P a r t i t i o n   l i n e . 
         f l o a t                               d y ;                         / /   P a r t i t i o n   l i n e . 
         f l o a t                               b b o x [ 2 ] [ 4 ] ;         / /   B o u n d i n g   b o x   f o r   e a c h   c h i l d . 
         u n s i g n e d   i n t                 c h i l d r e n [ 2 ] ;       / /   I f   N F _ S U B S E C T O R   i t ' s   a   s u b s e c t o r . 
 }   n o d e _ t ; 
 
 # e n d i f 
 