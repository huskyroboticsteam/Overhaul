from setuptools import find_packages, setup

package_name = 'pixhawk_imu_gps_publisher'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='powerlax',
    maintainer_email='raovishruth@gmail.com',
    description='Publishes the IMU and GPS data from the Pixhawk.',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'talker = pixhawk_imu_gps_publisher.publisher:main',
        ],
    },
)
