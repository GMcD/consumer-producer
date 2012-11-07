
Out of Source Build Notes:
1. Adjust paths to Linux or Windows - replace the placeholder /folderondevmachine/ConsumerProducer with destination.
2. Install cmake-2.8
3. Install boost-1.46 

4. mkdir /folderondevmachine/ConsumerProducer
5. cp *.* /folderondevmachine/ConsumerProducer
6. mkdir /folderondevmachine/build
7. cd /folderondevmachine/build
8. cmake ../ConsumerProducer
9. [ msbuild ConProd.sln | make ]

